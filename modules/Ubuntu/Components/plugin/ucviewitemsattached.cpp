/*
 * Copyright 2014 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ucunits.h"
#include "uctheme.h"
#include "uclistitem.h"
#include "uclistitem_p.h"
#include "propertychange_p.h"
#include "quickutils.h"
#include "i18n.h"
#include <QtQuick/private/qquickflickable_p.h>
#include <QtQml/QQmlInfo>

/*
 * The properties are attached to the ListItem's parent item or to its closest
 * Flickable parent, when embedded in ListView or Flickable. There will be only
 * one attached property per Flickable for all embedded child ListItems, enabling
 * in this way the controlling of the interactive flag of the Flickable and all
 * its ascendant Flickables.
 */
UCViewItemsAttachedPrivate::UCViewItemsAttachedPrivate(UCViewItemsAttached *qq)
    : q_ptr(qq)
    , listView(0)
    , globalDisabled(false)
    , selectable(false)
    , draggable(false)
{
}

UCViewItemsAttachedPrivate::~UCViewItemsAttachedPrivate()
{
    clearChangesList();
    clearFlickablesList();
}

// disconnect all flickables
void UCViewItemsAttachedPrivate::clearFlickablesList()
{
    Q_Q(UCViewItemsAttached);
    Q_FOREACH(const QPointer<QQuickFlickable> &flickable, flickables) {
        if (flickable.data())
        QObject::disconnect(flickable.data(), &QQuickFlickable::movementStarted,
                            q, &UCViewItemsAttached::unbindItem);
    }
    flickables.clear();
}

// connect all flickables
void UCViewItemsAttachedPrivate::buildFlickablesList()
{
    Q_Q(UCViewItemsAttached);
    QQuickItem *item = qobject_cast<QQuickItem*>(q->parent());
    if (!item) {
        return;
    }
    clearFlickablesList();
    while (item) {
        QQuickFlickable *flickable = qobject_cast<QQuickFlickable*>(item);
        if (flickable) {
            QObject::connect(flickable, &QQuickFlickable::movementStarted,
                             q, &UCViewItemsAttached::unbindItem);
            flickables << flickable;
        }
        item = item->parentItem();
    }
}

void UCViewItemsAttachedPrivate::clearChangesList()
{
    // clear property change objects
    Q_Q(UCViewItemsAttached);
    Q_FOREACH(PropertyChange *change, changes) {
        // deleting PropertyChange will restore the saved property
        // to its original binding/value
        delete change;
    }
    changes.clear();
}

void UCViewItemsAttachedPrivate::buildChangesList(const QVariant &newValue)
{
    // collect all ascendant flickables
    Q_Q(UCViewItemsAttached);
    QQuickItem *item = qobject_cast<QQuickItem*>(q->parent());
    if (!item) {
        return;
    }
    clearChangesList();
    while (item) {
        QQuickFlickable *flickable = qobject_cast<QQuickFlickable*>(item);
        if (flickable) {
            PropertyChange *change = new PropertyChange(item, "interactive");
            PropertyChange::setValue(change, newValue);
            changes << change;
        }
        item = item->parentItem();
    }
}

/*!
 * \qmltype ViewItems
 * \instantiates UCViewItemsAttached
 * \inqmlmodule Ubuntu.Components 1.2
 * \ingroup unstable-ubuntu-listitems
 * \since Ubuntu.Components 1.2
 * \brief A set of properties attached to the ListItem's parent item or ListView.
 */
UCViewItemsAttached::UCViewItemsAttached(QObject *owner)
    : QObject(owner)
    , d_ptr(new UCViewItemsAttachedPrivate(this))
{
    if (owner->inherits("QQuickListView")) {
        d_ptr->listView = static_cast<QQuickFlickable*>(owner);
    }
}

UCViewItemsAttached::~UCViewItemsAttached()
{
}

UCViewItemsAttached *UCViewItemsAttached::qmlAttachedProperties(QObject *owner)
{
    return new UCViewItemsAttached(owner);
}

// register item to be rebound
bool UCViewItemsAttached::listenToRebind(UCListItem *item, bool listen)
{
    // we cannot bind the item until we have an other one bound
    bool result = false;
    Q_D(UCViewItemsAttached);
    if (listen) {
        if (d->boundItem.isNull() || (d->boundItem == item)) {
            d->boundItem = item;
            // rebuild flickable list
            d->buildFlickablesList();
            result = true;
        }
    } else if (d->boundItem == item) {
        d->boundItem.clear();
        result = true;
    }
    return result;
}

// reports true if any of the ascendant flickables is moving
bool UCViewItemsAttached::isMoving()
{
    Q_D(UCViewItemsAttached);
    Q_FOREACH(const QPointer<QQuickFlickable> &flickable, d->flickables) {
        if (flickable && flickable->isMoving()) {
            return true;
        }
    }
    return false;
}

// returns true if the given ListItem is bound to listen on moving changes
bool UCViewItemsAttached::isBoundTo(UCListItem *item)
{
    Q_D(UCViewItemsAttached);
    return d->boundItem == item;
}

/*
 * Disable/enable interactive flag for the ascendant flickables. The item is used
 * to detect whether the same item is trying to enable the flickables which disabled
 * it before. The enabled/disabled states are not equivalent to the enabled/disabled
 * state of the interactive flag.
 * When disabled, always the last item disabling will be kept as active disabler,
 * and only the active disabler can enable (restore) the interactive flag state.
 */
void UCViewItemsAttached::disableInteractive(UCListItem *item, bool disable)
{
    Q_D(UCViewItemsAttached);
    if (disable) {
        // disabling or re-disabling
        d->disablerItem = item;
        if (d->globalDisabled == disable) {
            // was already disabled, leave
            return;
        }
        d->globalDisabled = true;
    } else if (d->globalDisabled && d->disablerItem == item) {
        // the one disabled it will enable
        d->globalDisabled = false;
        d->disablerItem.clear();
    } else {
        // !disabled && (!globalDisabled || item != d->disablerItem)
        return;
    }
    if (disable) {
        // (re)build changes list with disabling the interactive value
        d->buildChangesList(false);
    } else {
        d->clearChangesList();
    }
}

void UCViewItemsAttached::unbindItem()
{
    Q_D(UCViewItemsAttached);
    if (d->boundItem) {
        // depending on content item's X coordinate, we either do animated or prompt rebind
        if (d->boundItem->contentItem()->x() != 0.0) {
            // content is not in origin, rebind
            UCListItemPrivate::get(d->boundItem.data())->_q_rebound();
        } else {
            // do some cleanup
            UCListItemPrivate::get(d->boundItem.data())->promptRebound();
        }
        d->boundItem.clear();
    }
    // clear binding list
    d->clearFlickablesList();
}

/*!
 * \qmlattachedproperty bool ViewItems::selectMode
 * The property drives whether list items are selectable or not. The property is
 * attached to the ListItem's parent or to the ListView/Flickable owning the
 * ListItems.
 *
 * When set, the default style implementation will show a check box on the leading
 * side hanving the content item pushed towards trailing side and dimmed. The checkbox
 * which will reflect and drive the \l {ListItem::selected}{selected} state.
 * Defaults to false.
 */
bool UCViewItemsAttachedPrivate::selectMode() const
{
    return selectable;
}
void UCViewItemsAttachedPrivate::setSelectMode(bool value)
{
    if (selectable == value) {
        return;
    }
    selectable = value;
    Q_Q(UCViewItemsAttached);
    Q_EMIT q->selectModeChanged();
}

/*!
 * \qmlattachedproperty list<int> ViewItems::selectedIndexes
 * The property is automatically attached to the ListItem's parent item, or to
 * the ListView when used with ListView. Contains the indexes of the ListItems
 * marked as selected. The indexes are model indexes when used in ListView, and
 * child indexes in other contexts.
 * \note Setting the ListItem's \l {ListItem::selected}{selected} property to \c
 * true will add the item index to the selection list automatically, and may
 * destroy the initial state of the selection. Therefore it is recommended to
 * drive the selection through the attached property rather through the \l
 * ListItem::selected property. \sa ListItem::selectable, ListItem::selected
 */
QList<int> UCViewItemsAttachedPrivate::selectedIndexes() const
{
    return selectedList;
}
void UCViewItemsAttachedPrivate::setSelectedIndexes(const QList<int> &list)
{
    if (selectedList == list) {
        return;
    }
    selectedList = list;
    Q_Q(UCViewItemsAttached);
    Q_EMIT q->selectedIndexesChanged();
}

void UCViewItemsAttachedPrivate::addSelectedItem(UCListItem *item)
{
    int index = UCListItemPrivate::get(item)->index();
    if (!selectedList.contains(index)) {
        selectedList.append(index);
        Q_EMIT q_ptr->selectedIndexesChanged();
    }
}
void UCViewItemsAttachedPrivate::removeSelectedItem(UCListItem *item)
{
    if (selectedList.removeAll(UCListItemPrivate::get(item)->index()) > 0) {
        Q_EMIT q_ptr->selectedIndexesChanged();
    }
}

bool UCViewItemsAttachedPrivate::isItemSelected(UCListItem *item)
{
    return selectedList.contains(UCListItemPrivate::get(item)->index());
}

/*!
 * \qmlattachedproperty bool ViewItems::dragMode
 * The property drives the dragging mode of the ListItems within a ListView. It
 * has no effect on any other parent of the ListItem.
 *
 * When set, ListItem content will be disabled and a panel will be shown enabling
 * the dragging mode. The items can be dragged by dragging this handler only.
 * The feature can be activated same time with \l ListItem::selectable.
 *
 * The panel is configured by the \l {ListItemStyle::dragHandlerDelegate}{dragHandlerDelegate}
 * component.
 *
 * \sa ListItemStyle::dragHandlerDelegate, draggingStarted
 */

/*!
 * \qmlattachedsignal ViewItems::draggingStarted(ListItemDrag event)
 * The signal is emitted when a ListItem dragging is started. \c event.from
 * specifies the index of the ListItem being dragged. \c event.minimumIndex and
 * \c event.maximumIndex configures the index interval the dragging of the item
 * is allowed. If set (meaning their value differs from -1), items cannot be
 * dragged outside of this region. The \c event.accept property, if set to false,
 * will cancel dragging operation. The other fields of the event (i.e. \c event.to
 * and \c event.direction) contain invalid data.
 * \qml
 * import QtQuick 2.3
 * import Ubuntu.Components 1.2
 *
 * ListView {
 *     width: units.gu(40)
 *     height: units.gu(40)
 *     model: ListModel {
 *         // initiate with random data
 *     }
 *     delegate: ListItem {
 *         // content
 *     }
 *
 *     ViewItems.dragMode: true
 *     ViewItems.onDraggingStarted: {
 *         if (event.from < 5) {
 *             // deny dragging on the first 5 element
 *             event.accept = false;
 *         } else if (event.from >= 5 && event.from <= 10) {
 *             // specify the interval
 *             event.minimumIndex = 5;
 *             event.maximumIndex = 10;
 *         }
 *     }
 * }
 * \endqml
 *
 * In the example above the first 5 items are not draggable, though drag handler
 * will still be shown for them. If the drag starts on item index 5, it will only
 * accept drag gesture downwards, respectively starting a drag on item index 10
 * will only allow dragging that element upwards. Every item dragged between
 * indexes 5 and 10 will be draggable both directions, however only till 5th or
 * 10th index. On the other hand, items dragged from index > 10 will be draggable
 * to any index, including the first 11 items. In order to avoid dragging those
 * items in between the first 11 items, the following change must be made:
 * \qml
 * import QtQuick 2.3
 * import Ubuntu.Components 1.2
 *
 * ListView {
 *     width: units.gu(40)
 *     height: units.gu(40)
 *     model: ListModel {
 *         // initiate with random data
 *     }
 *     delegate: ListItem {
 *         // content
 *     }
 *
 *     ViewItems.dragMode: true
 *     ViewItems.onDraggingStarted: {
 *         if (event.from < 5) {
 *             // deny dragging on the first 5 element
 *             event.accept = false;
 *         } else if (event.from >= 5 && event.from <= 10) {
 *             // specify the interval
 *             event.minimumIndex = 5;
 *             event.maximumIndex = 10;
 *         } else {
 *             // prevent dragging to the first 11 items area
 *             event.minimumIndex = 11;
 *         }
 *     }
 * }
 * \endqml
 *
 * \note None of the above examples will move the dragged item. In order that to
 * happen, you must implement \l draggingUpdated signal and move the model data.
 * \note Implementing the signal handler is not mandatory and should only happen
 * if restrictions on the drag must be applied.
 */

/*!
 * \qmlattachedsignal ViewItems::draggingUpdated(ListItemDrag event)
 * The signal is emitted when the list item from \c event.from index has been
 * dragged over to \c event.to, and a move operation is possible. Implementations
 * \b {must move the model data} between these indexes. If the move is not acceptable,
 * it can be cancelled by setting \c event.accept to \c false, in which case the
 * dragged item will stay on its last moved position or will snap back to its
 * previous or original place, depending whether the drag was sent during the
 * drag or as a result of a drop gesture. The direction of the drag is given in the
 * \c event.direction property. Extending the example from \l draggingStarted, an
 * implementation of a live dragging would look as follows
 * \qml
 * import QtQuick 2.3
 * import Ubuntu.Components 1.2
 *
 * ListView {
 *     width: units.gu(40)
 *     height: units.gu(40)
 *     model: ListModel {
 *         // initiate with random data
 *     }
 *     delegate: ListItem {
 *         // content
 *     }
 *
 *     ViewItems.dragMode: true
 *     ViewItems.onDraggingStarted: {
 *         if (event.from < 5) {
 *             // deny dragging on the first 5 element
 *             event.accept = false;
 *         } else if (event.from >= 5 && event.from <= 10) {
 *             // specify the interval
 *             event.minimumIndex = 5;
 *             event.maximumIndex = 10;
 *         } else if (event.from > 10) {
 *             // prevent dragging to the first 11 items area
 *             event.minimumIndex = 11;
 *         }
 *     }
 *     ViewItems.onDraggingUpdated: {
 *         model.move(event.from, event.to, 1);
 *     }
 * }
 * \endqml
 */
bool UCViewItemsAttachedPrivate::dragMode() const
{
    return draggable;
}
void UCViewItemsAttachedPrivate::setDragMode(bool value)
{
    if (draggable == value) {
        return;
    }
    Q_Q(UCViewItemsAttached);
    if (value) {
        /*
         * The dragging works only if the ListItem is used inside a ListView, and the
         * model used is a list, a ListModel or a derivate of QAbstractItemModel. Do
         * not enable dragging if these conditions are not fulfilled.
         */
        if (!listView) {
            qmlInfo(q->parent()) << UbuntuI18n::instance().tr("dragging mode requires ListView");
            return;
        }
        QVariant modelValue = listView->property("model");
        if (!modelValue.isValid()) {
            return;
        }
        if (modelValue.type() == QVariant::Int || modelValue.type() == QVariant::Double) {
            qmlInfo(listView) << UbuntuI18n::instance().tr("model must be a list, ListModel or a derivate of QAbstractItemModel");
            return;
        }
    }
    draggable = value;
    Q_EMIT q->dragModeChanged();
}