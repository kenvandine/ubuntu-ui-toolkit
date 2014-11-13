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
#include <QtQuick/private/qquickflickable_p.h>

/*
 * The properties are attached to the ListItem's parent item or to its closest
 * Flickable parent, when embedded in ListView or Flickable. There will be only
 * one attached property per Flickable for all embedded child ListItems, enabling
 * in this way the controlling of the interactive flag of the Flickable and all
 * its ascendant Flickables.
 */
UCListItemAttachedPrivate::UCListItemAttachedPrivate(UCListItemAttached *qq)
    : q_ptr(qq)
    , globalDisabled(false)
{
}

UCListItemAttachedPrivate::~UCListItemAttachedPrivate()
{
    clearInteractiveList();
}

void UCListItemAttachedPrivate::connectFlickables()
{
    Q_Q(UCListItemAttached);
    QQuickItem *item = qobject_cast<QQuickItem*>(q->parent());
    if (!item) {
        return;
    }

    Q_FOREACH(QQuickFlickable *flickable, flickableList) {
        QObject::disconnect(flickable, &QQuickFlickable::movementStarted,
                            q, &UCListItemAttached::unbindItem);
    }
    flickableList.clear();

    while (item) {
        QQuickFlickable *flickable = qobject_cast<QQuickFlickable*>(item);
        if (flickable) {
            flickableList << flickable;
            QObject::connect(flickable, &QQuickFlickable::movementStarted,
                             q, &UCListItemAttached::unbindItem);
        }
        item = item->parentItem();
    }
}

void UCListItemAttachedPrivate::clearInteractiveList()
{
    // clear property change objects
    Q_Q(UCListItemAttached);
    Q_FOREACH(PropertyChange *change, interactiveList) {
        // deleting PropertyChange will restore the saved property
        // to its original binding/value
        delete change;
    }
    interactiveList.clear();
}

void UCListItemAttachedPrivate::buildInteractiveList()
{
    Q_Q(UCListItemAttached);
    QQuickItem *item = qobject_cast<QQuickItem*>(q->parent());
    if (!item) {
        return;
    }
    clearInteractiveList();
    while (item) {
        QQuickFlickable *flickable = qobject_cast<QQuickFlickable*>(item);
        if (flickable) {
            PropertyChange *change = new PropertyChange(item, "interactive");
            interactiveList << change;
        }
        item = item->parentItem();
    }

}

UCListItemAttached::UCListItemAttached(QObject *owner)
    : QObject(owner)
    , d_ptr(new UCListItemAttachedPrivate(this))
{
}

UCListItemAttached::~UCListItemAttached()
{
}

// register item to be rebount
bool UCListItemAttached::listenToRebind(UCListItem *item, bool listen)
{
    // we cannot bind the item until we have an other one bount
    bool result = false;
    Q_D(UCListItemAttached);
    if (listen) {
        if (d->bountItem.isNull() || (d->bountItem == item)) {
            if (d->bountItem.isNull()) {
                d->connectFlickables();
            }
            d->bountItem = item;
            result = true;
        }
    } else if (d->bountItem == item) {
        d->bountItem.clear();
        result = true;
    }
    return result;
}

// reports true if any of the ascendant flickables is moving
bool UCListItemAttached::isMoving()
{
    Q_D(UCListItemAttached);
    Q_FOREACH(QQuickFlickable *flickable, d->flickableList) {
        if (flickable->isMoving()) {
            return true;
        }
    }
    return false;
}

// returns true if the given ListItem is bount to listen on moving changes
bool UCListItemAttached::isBountTo(UCListItem *item)
{
    Q_D(UCListItemAttached);
    return d->bountItem == item;
}

/*
 * Disable/enable interactive flag for the ascendant flickables. The item is used
 * to detect whether the sam item is trying to enable the flickables which disabled
 * it before. The enabled/disabled states are not equivalent to the enabled/disabled
 * state of the interactive flag.
 * When disabled, always the last item disabling will be kept as active disabler,
 * and only the active disabler can enable (restore) the interactive flag state.
 */
void UCListItemAttached::disableInteractive(UCListItem *item, bool disable)
{
    Q_D(UCListItemAttached);
    if ((d->globalDisabled && disable) || (!d->globalDisabled && disable)) {
        // disabling or re-disabling
        d->disablerItem = item;
        if (d->globalDisabled == disable) {
            // was already disabled, leave
            return;
        }
        d->globalDisabled = disable;
    } else if (d->globalDisabled && !disable && d->disablerItem == item) {
        // the one disabled it will enable
        d->globalDisabled = disable;
        d->disablerItem.clear();
    } else {
        // none of the above, leave
        return;
    }
    // build interactive change list
    d->buildInteractiveList();
    Q_FOREACH(PropertyChange *change, d->interactiveList) {
        if (disable) {
            PropertyChange::setValue(change, false);
        } else {
            PropertyChange::restore(change);
        }
    }
}

void UCListItemAttached::unbindItem()
{
    Q_D(UCListItemAttached);
    if (d->bountItem) {
        // depending on content item's X coordinate, we either rebound or prompt drop
        if (d->bountItem->contentItem()->x() != 0.0) {
            // content is not in origin, rebind
            UCListItemPrivate::get(d->bountItem.data())->_q_rebound();
        } else {
            // do some cleanup
            UCListItemPrivate::get(d->bountItem.data())->promptRebound();
        }
        d->bountItem.clear();
    }
}

/*!
 * \qmlattachedproperty list<int> ListItem::selectedIndexes
 * The property is automatically attached to the ListItem's parent item, or to
 * the ListView when used with ListView. Contains the indexes of the ListItems
 * marked as selected. The indexes are model indexes when used in ListView, and
 * child indexes in other contexts.
 * \note Setting the ListItem's \l selected property to \c true will add the
 * item index to the selection list automatically, and may destroy the initial
 * state of the selection. Therefore it is recommended to drive the selection
 * through the attached property rather through the \l ListItem::selected property.
 * \sa ListItem::selectable, ListItem::selected
 */
QList<int> UCListItemAttached::selectedIndexes() const
{
    Q_D(const UCListItemAttached);
    return d->indexList;
}
void UCListItemAttached::setSelectedIndexes(const QList<int> &list)
{
    Q_D(UCListItemAttached);
    if (d->indexList == list) {
        return;
    }
    d->indexList = list;
    Q_EMIT selectedIndexesChanged();
}

void UCListItemAttachedPrivate::addSelectedItem(UCListItem *item)
{
    int index = UCListItemPrivate::get(item)->index();
    if (!indexList.contains(index)) {
        indexList.append(index);
        Q_EMIT q_ptr->selectedIndexesChanged();
    }
}
void UCListItemAttachedPrivate::removeSelectedItem(UCListItem *item)
{
    indexList.removeAll(UCListItemPrivate::get(item)->index());
    Q_EMIT q_ptr->selectedIndexesChanged();
}

bool UCListItemAttachedPrivate::isItemSelected(UCListItem *item)
{
    return indexList.contains(UCListItemPrivate::get(item)->index());
}


