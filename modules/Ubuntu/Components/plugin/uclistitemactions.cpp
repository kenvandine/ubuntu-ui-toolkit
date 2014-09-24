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

#include "uclistitemactions.h"
#include "uclistitemactions_p.h"
#include "uclistitem_p.h"
#include "quickutils.h"
#include "i18n.h"
#include "plugin.h"
#include <QtQml/QQmlInfo>
#include <QtQuick/private/qquickitem_p.h>
#include "ucaction.h"


UCListItemActionsAttached::UCListItemActionsAttached(QObject *parent)
    : QObject(parent)
    , m_listItemActions(0)
{
}

UCListItemActionsAttached::~UCListItemActionsAttached()
{
}

/*!
 * \qmlattachedproperty list<real> ListItemActions::snapStops
 * The property configures the snapping stops on custom panel implementations.
 * The values set are taken into account only on custom panels, those are omitted
 * when the default panel is used.
 */

/*!
 * \qmlattachedproperty ListItemActions ListItemActions::instance
 * The property holds the instance of the ListItemActions the \l panelItem is
 * attached to.
 */
void UCListItemActionsAttached::setList(UCListItemActions *list)
{
    if (list == m_listItemActions) {
        return;
    }
    m_listItemActions = list;
    Q_EMIT containerChanged();
}

/*!
 * \qmlattachedproperty real ListItemActions::offsetVisible
 * The property returns the offset the \l panelItem is visible. This can be used
 * to do different animations on the panel and on the action visualizations.
 */
qreal UCListItemActionsAttached::offsetVisible()
{
    Q_ASSERT(m_listItemActions);
    return UCListItemActionsPrivate::get(m_listItemActions)->offsetDragged;
}

UCListItemActionsPrivate::UCListItemActionsPrivate()
    : QObjectPrivate()
    , status(UCListItemActions::Disconnected)
    , delegate(0)
    , customPanel(0)
    , panelItem(0)
    , backgroundColor(Qt::transparent)
    , foregroundColor(Qt::transparent)
    , optionSlotWidth(0.0)
    , offsetDragged(0.0)
    , optionsVisible(0)
{
}
UCListItemActionsPrivate::~UCListItemActionsPrivate()
{
}

void UCListItemActionsPrivate::_q_handlePanelDrag()
{
    UCListItem *listItem = qobject_cast<UCListItem*>(panelItem->parentItem());
    if (!listItem) {
        return;
    }

    Q_Q(UCListItemActions);
    offsetDragged = (status == UCListItemActions::Leading) ? panelItem->width() + panelItem->x() :
                         listItem->width() - panelItem->x();
    if (offsetDragged < 0.0) {
        offsetDragged = 0.0;
    }
    if (optionSlotWidth > 0.0) {
        optionsVisible = (int)trunc(offsetDragged / optionSlotWidth);
    }
}

void UCListItemActionsPrivate::_q_handlePanelWidth()
{
    // check how many options are visible && enabled
    // FIXME: use Actions API when moved to C++
    int count = 0;
    for (int i = 0; i < actions.count(); i++) {
        if (actions[i]->property("visible").toBool() && actions[i]->property("enabled").toBool()) {
            count++;
        }
    }
    optionSlotWidth = panelItem->width() / count;
    _q_handlePanelDrag();
}

bool UCListItemActionsPrivate::connectToListItem(UCListItemActions *actions, UCListItem *listItem, bool leading)
{
    UCListItemActionsPrivate *_this = get(actions);
    if (!_this || !_this->createPanelItem() || isConnectedTo(actions, listItem)) {
        return isConnectedTo(actions, listItem);
    }
    // check if the panel is still connected to a ListItem
    // this may happen if there is a swipe over an other item while the previous
    // one is rebounding
    if (_this->panelItem->parentItem()) {
        // set the requesting listItem as queuedItem
        _this->queuedItem = listItem;
        return false;
    }
//    _this->panelItem->setProperty("listItemIndex", UCListItemPrivate::get(listItem)->index);
    _this->attachedObject()->setItemIndex(UCListItemPrivate::get(listItem)->index);
    _this->panelItem->setParentItem(listItem);
    _this->offsetDragged = 0.0;

    // if the panel has selected() signal, connect to it
    if (_this->panelItem->metaObject()->indexOfMethod("selected") >= 0) {
        QObject::connect(_this->panelItem, SIGNAL(selected()), _this->panelItem->parentItem(), SLOT(_q_rebound()));
    }
    _this->status = (leading) ?  UCListItemActions::Leading :  UCListItemActions::Trailing;
    Q_EMIT actions->statusChanged();
    Q_EMIT actions->connectedItemChanged();
    return true;
}

void UCListItemActionsPrivate::disconnectFromListItem(UCListItemActions *actions)
{
    UCListItemActionsPrivate *_this = get(actions);
    if (!_this || !_this->panelItem || !_this->panelItem->parentItem()) {
        return;
    }

    if (_this->panelItem->metaObject()->indexOfMethod("selected") >= 0) {
        QObject::disconnect(_this->panelItem, SIGNAL(selected()), _this->panelItem->parentItem(), SLOT(_q_rebound()));
    }
    _this->attachedObject()->setItemIndex(-1);
    _this->panelItem->setParentItem(0);
    _this->status = UCListItemActions::Disconnected;
    Q_EMIT actions->statusChanged();
    Q_EMIT actions->connectedItemChanged();
    // if there was a queuedItem, make it grab the actions list
    if (_this->queuedItem) {
        UCListItemPrivate::get(_this->queuedItem.data())->grabPanel(actions, true);
        // remove item from queue
        _this->queuedItem.clear();
    }
}

bool UCListItemActionsPrivate::isConnectedTo(UCListItemActions *actions, UCListItem *listItem)
{
    UCListItemActionsPrivate *_this = get(actions);
    return _this && _this->panelItem &&
            (_this->status != UCListItemActions::Disconnected) &&
            (_this->panelItem->parentItem() == listItem);
}

void UCListItemActionsPrivate::drag(UCListItemActions *options, UCListItem *listItem, bool started)
{
    UCListItemActionsPrivate *_this = get(options);
    if (!_this || !_this->panelItem || !isConnectedTo(options, listItem)) {
        return;
    }
    if (started) {
        Q_EMIT _this->attachedObject()->dragStarted();
    } else {
        Q_EMIT _this->attachedObject()->dragEnded();
    }
}

qreal UCListItemActionsPrivate::snap(UCListItemActions *options)
{
    UCListItemActionsPrivate *_this = get(options);
    if (!_this || !_this->panelItem) {
        return 0.0;
    }
    qreal result = 0.0;
    if (_this->customPanel) {
        // check if the attached property has snapStops set
        UCListItemActionsAttached *attached = _this->attachedObject();
        if (attached && (attached->snapStops().count() > 0)) {
            QList<qreal> stops(attached->snapStops());
            // append the full width to the stops so we can snap to it too
            stops.append(_this->panelItem->width());
            for (int i = 0; i < stops.count(); i++) {
                if (stops[i] < _this->offsetDragged) {
                    continue;
                }
                // get the interval
                qreal min = (i > 1) ? stops[i - 1] : 0.0;
                qreal max = stops[i];
                if (_this->offsetDragged - min > (max - min) / 2) {
                    result = max;
                    break;
                }
            }
        } else {
            // no snapping, return the offset as is.
            result = _this->offsetDragged;
        }
    } else {
        // default panel, do snapping based on the actions
        qreal ratio = _this->offsetDragged / _this->optionSlotWidth;
        int visible = _this->optionsVisible;
        if (ratio > 0.0 && (ratio - trunc(ratio)) > 0.5) {
            visible++;
        }
        result = visible * _this->optionSlotWidth;
    }
    return result * (_this->status ==  UCListItemActions::Leading ? 1 : -1);
}

UCListItemActionsAttached *UCListItemActionsPrivate::attachedObject()
{
    if (!panelItem) {
        return 0;
    }
    return static_cast<UCListItemActionsAttached*>(
                qmlAttachedPropertiesObject<UCListItemActions>(panelItem, false));
}

QQuickItem *UCListItemActionsPrivate::createPanelItem()
{
    if (panelItem) {
        return panelItem;
    }
    Q_Q(UCListItemActions);
    UCListItemActionsAttached *attached = 0;
    if (customPanel) {
        panelItem = qobject_cast<QQuickItem*>(customPanel->beginCreate(qmlContext(q)));
        if (panelItem) {
            QQml_setParent_noEvent(panelItem, q);
            // create attached property!
            attached = static_cast<UCListItemActionsAttached*>(
                        qmlAttachedPropertiesObject<UCListItemActions>(panelItem));
            if (attached) {
                attached->setList(q);
            }
            customPanel->completeCreate();
        }
    } else {
        QUrl panelDocument = UbuntuComponentsPlugin::pluginUrl().
                resolved(QUrl::fromLocalFile("ListItemPanel.qml"));
        QQmlComponent component(qmlEngine(q), panelDocument);
        if (!component.isError()) {
            panelItem = qobject_cast<QQuickItem*>(component.beginCreate(qmlContext(q)));
            if (panelItem) {
                QQml_setParent_noEvent(panelItem, q);
                // create attached property!
                attached = static_cast<UCListItemActionsAttached*>(
                            qmlAttachedPropertiesObject<UCListItemActions>(panelItem));
                if (attached) {
                    attached->setList(q);
                }
                component.completeCreate();
            }
        } else {
            qmlInfo(q) << component.errorString();
        }
    }
    if (panelItem) {
        Q_EMIT q->panelItemChanged();

        // calculate option's slot size
        offsetDragged = 0.0;
        optionsVisible = 0;
        _q_handlePanelWidth();
        // connect to panel to catch dragging
        QObject::connect(panelItem, SIGNAL(widthChanged()), q, SLOT(_q_handlePanelWidth()));
        QObject::connect(panelItem, SIGNAL(xChanged()), q, SLOT(_q_handlePanelDrag()));
        QObject::connect(panelItem, SIGNAL(xChanged()), attached, SIGNAL(offsetVisibleChanged()));
    }
    return panelItem;
}

/*!
 * \qmltype ListItemActions
 * \instantiates UCListItemActions
 * \inherits QtQObject
 * \inqmlmodule Ubuntu.Components 1.2
 * \since Ubuntu.Components 1.2
 * \ingroup unstable-ubuntu-listitems
 * \brief Provides configuration for actions to be added to a ListItem.
 *
 * ListItem accepts actions that can be configured to appear when tugged to left
 * or right. The API does not limit the number of actions to be assigned for leading
 * or trailing actions, however the design constrains are allowing a maximum of
 * 1 action on leading- and a maximum of 3 actions on trailing side of the ListItem.
 *
 * The \l actions are Action instances or elements derived from Action. The default
 * visualization of the actions can be overridden using the \l delegate property,
 * and the default implementation uses the \c name property of the Action.
 *
 * The leading and trailing actions are placed on \l panelItem, which is created
 * the first time the actions are accessed. The colors of the panel is taken from
 * the theme's palette.
 *
 * When tugged, panels reveal the actions one by one. In case an action is revealed
 * more than 50%, the action will be snapped and revealed completely. This is also
 * valid for the case when the action is visible less than 50%, in which case the
 * action is hidden. Actions can be triggered by tapping.
 *
 * \note You can use the same ListItemActions for leading and for trailing actions
 * the same time only if the instance is used by different groups of list items,
 * where one group uses it as leading and other group as trailing. In any other
 * circumstances use separate ListItemActions for leading and trailing actions.
 * \qml
 * import QtQuick 2.2
 * import Ubuntu.Components 1.2
 * MainView {
 *     width: units.gu(40)
 *     height: units.gu(71)
 *
 *     ListItemActions {
 *         id: sharedActions
 *         actions: [
 *             Action {
 *                 iconName: "search"
 *             },
 *             Action {
 *                 iconName: "edit"
 *             },
 *             Action {
 *                 iconName: "copy"
 *             }
 *         ]
 *     }
 *
 *     Column {
 *         ListItem {
 *             leadingActions: sharedActions
 *         }
 *         UbuntuListView {
 *             anchors.fill: parent
 *             model: 10000
 *             delegate: ListItem {
 *                 trailingActions: sharedActions
 *             }
 *         }
 *     }
 * }
 * \endqml
 *
 * \section3 Using with ListViews
 * When used with views, or when the amount of items of same kind to be created
 * is huge, it is recommended to use cached ListItemActions instances to reduce
 * creation time and to be able to handle rebounding and flicking properly. If
 * each ListItem crteates its own ListItemActions instance the Flickable view may
 * be blocked and action visualization will also break.
 * \qml
 * import QtQuick 2.2
 * import Ubuntu.Components 1.2
 *
 * MainView {
 *     width: units.gu(40)
 *     height: units.gu(71)
 *
 *     UbuntuListView {
 *         anchors.fill: parent
 *         model: 10000
 *         ListItemActions {
 *             id: commonActions
 *             actions: [
 *                 Action {
 *                     iconName: "search"
 *                 },
 *                 Action {
 *                     iconName: "edit"
 *                 },
 *                 Action {
 *                     iconName: "copy"
 *                 }
 *             ]
 *         }
 *         delegate: ListItem {
 *             trailingActions: commonActions
 *         }
 *     }
 * }
 * \endqml
 */

UCListItemActions::UCListItemActions(QObject *parent)
    : QObject(*(new UCListItemActionsPrivate), parent)
{
}
UCListItemActions::~UCListItemActions()
{
}

UCListItemActionsAttached *UCListItemActions::qmlAttachedProperties(QObject *owner)
{
    return new UCListItemActionsAttached(owner);
}

/*!
 * \qmlproperty Component ListItemActions::delegate
 * Custom delegate which overrides the default one used by the ListItem. If the
 * value is null, the default delegate will be used.
 *
 * ListItemActions provides the \c action context property which contains the
 * Action instance currently visualized. Using this property delegates can access
 * the information to be visualized. The trigger is handled by the \l panelItem
 * therefore only visualization is needed by the custom delegates. The other
 * context property exposed to delegates is the \c index, which specifies the
 * index of the action visualized.
 *
 * The delegate height is set automatically by the panelItem, and the width value
 * is clamped between height and the maximum width of the list item divided by the
 * number of actions in the list.
 * \qml
 * import QtQuick 2.2
 * import Ubuntu.Components 1.2
 *
 * MainView {
 *     width: units.gu(40)
 *     height: units.gu(71)
 *
 *     UbuntuListView {
 *         anchors.fill: parent
 *         model: 50
 *         delegate: ListItem {
 *             trailingActions: optionsList
 *         }
 *         ListItemActions {
 *             id: optionsList
 *             delegate: Column {
 *                 width: height + units.gu(2)
 *                 Icon {
 *                     name: action.iconName
 *                     width: units.gu(3)
 *                     height: width
 *                     color: "blue"
 *                     anchors.horizontalCenter: parent.horizontalCenter
 *                 }
 *                 Label {
 *                     text: option.text + "#" + index
 *                     width: parent.width
 *                     horizontalAlignment: Text.AlignHCenter
 *                 }
 *             }
 *             actions: Action {
 *                 iconName: "starred"
 *                 text: "Star"
 *             }
 *         }
 *     }
 * }
 * \endqml
 * \note Putting a Rectangle in the delegate can be used to override the color
 * of the panel.
 *
 * Defaults to null.
 */
QQmlComponent *UCListItemActions::delegate() const
{
    Q_D(const UCListItemActions);
    return d->delegate;
}
void UCListItemActions::setDelegate(QQmlComponent *delegate)
{
    Q_D(UCListItemActions);
    if (d->delegate == delegate) {
        return;
    }
    d->delegate = delegate;
    Q_EMIT delegateChanged();
}

/*!
 * \qmlproperty Component ListItemActions::customPanel
 * The property configures the component the \l panelItem is created from. Defaults
 * to \c null.
 *
 * When a custom component is set, action triggering, snapping, coloring and sizing
 * must be handled by the custom component. The \l ListItemActions instance the
 * panel belongs to can be accessed through the \c ListItemActions.container
 * attached property and the amount of width visible is reported by the \c
 * ListItemActions.offsetVisible attached property. Snapping can be controlled by
 * the \c ListItemActions.snapStops array. If no value is set, no snapping will be
 * performed.
 */
QQmlComponent *UCListItemActions::customPanel() const
{
    Q_D(const UCListItemActions);
    return d->customPanel;
}
void UCListItemActions::setCustomPanel(QQmlComponent *panel)
{
    Q_D(UCListItemActions);
    if (d->customPanel == panel) {
        return;
    }
    // delete previous panel before we set the new one
    if (d->panelItem) {
        d->queuedItem.clear();
        UCListItem *listItem = static_cast<UCListItem*>(d->panelItem->parentItem());
        UCListItemPrivate *pListItem = UCListItemPrivate::get(listItem);
        // prompt rebounding and disconnect
        pListItem->cleanup();
        delete d->panelItem;
        d->panelItem = 0;
    }
    d->customPanel = panel;
    Q_EMIT customPanelChanged();
}

/*!
 * \qmlproperty list<Action> ListItemActions::actions
 * The property holds the actions to be displayed. It can hold instances cached or
 * declared in place. An example of cached actions:
 * \qml
 * ListItemActions {
 *     id: cacedActions
 *     actions: [
 *         copyAction, searchAction, cutAction
 *     ]
 * }
 * \endqml
 */
QQmlListProperty<UCAction> UCListItemActions::actions()
{
    Q_D(UCListItemActions);
    return QQmlListProperty<UCAction>(this, d->actions);
}

/*!
 * \qmlproperty Item ListItemActions::panelItem
 * The property presents the Item holding the visualized actions. The panel is
 * created when used the first time.
 */
QQuickItem *UCListItemActions::panelItem() const
{
    Q_D(const UCListItemActions);
    return d->panelItem;
}

/*!
 * \qmlproperty enum ListItemActions::status
 * \readonly
 * The property holds the status of the ListItemActions, whether is connected
 * as leading or as trailing option list to a \l ListItem. Possible valueas are:
 * \list A
 *  \li \b \c Disconnected - default, the options list is not connected to any \l ListItem
 *  \li \b \c LeadingOptions - the options list is connected as leading list
 *  \li \b \c TrailingOptions - the options list is connected as trailing list
 * \endlist
 * \sa connectedItem
 */
UCListItemActions::Status UCListItemActions::status() const
{
    Q_D(const UCListItemActions);
    return d->status;
}

/*!
 * \qmlproperty ListItem ListItemActions::connectedItem
 * \readonly
 * The property holds the \l ListItem the options list is connected to. It is
 * null by default and when the status is \c Disconnected.
 * \sa status
 */
UCListItem *UCListItemActions::connectedItem() const
{
    Q_D(const UCListItemActions);
    return d->panelItem ? qobject_cast<UCListItem*>(d->panelItem->parentItem()) : 0;
}
/*!
 * \internal
 * \qmlproperty list<QtObject> ListItemActions::data
 * \default
 * The property holds any additional content added to the ListItemActions.
 */
QQmlListProperty<QObject> UCListItemActions::data()
{
    Q_D(UCListItemActions);
    return QQmlListProperty<QObject>(this, d->data);
}

/*!
 * \qmlproperty color ListItemActions::backgroundColor
 * The property overrides the default colouring of the \l panelItem.
 */
QColor UCListItemActions::backgroundColor() const
{
    Q_D(const UCListItemActions);
    return d->backgroundColor;
}
void UCListItemActions::setBackgroundColor(const QColor &color)
{
    Q_D(UCListItemActions);
    if (d->backgroundColor == color) {
        return;
    }
    d->backgroundColor = color;
    Q_EMIT backgroundColorChanged();
}

/*!
 * \qmlproperty color ListItemActions::foregroundColor
 * The property overrides the default colouring of the icons or texts in the
 * options visualization.
 */
QColor UCListItemActions::foregroundColor() const
{
    Q_D(const UCListItemActions);
    return d->foregroundColor;
}
void UCListItemActions::setForegroundColor(const QColor &color)
{
    Q_D(UCListItemActions);
    if (d->foregroundColor == color) {
        return;
    }
    d->foregroundColor = color;
    Q_EMIT foregroundColorChanged();
}

#include "moc_uclistitemactions.cpp"
