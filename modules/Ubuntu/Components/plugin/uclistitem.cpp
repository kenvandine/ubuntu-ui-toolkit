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
#include "uclistitemactions.h"
#include "uclistitemactions_p.h"
#include "ucubuntuanimation.h"
#include "propertychange_p.h"
#include "i18n.h"
#include "quickutils.h"
#include "plugin.h"
#include <QtQml/QQmlInfo>
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickflickable_p.h>
#include <QtQuick/private/qquickpositioners_p.h>
#include <QtQuick/private/qquickmousearea_p.h>
#include <QtGui/QGuiApplication>
#include <QtGui/QStyleHints>

#define MIN(x, y)           ((x < y) ? x : y)
#define MAX(x, y)           ((x > y) ? x : y)
#define CLAMP(v, min, max)  (min <= max) ? MAX(min, MIN(v, max)) : MAX(max, MIN(v, min))

QColor getPaletteColor(const char *profile, const char *color)
{
    QColor result;
    QObject *palette = UCTheme::instance().palette();
    if (palette) {
        QObject *paletteProfile = palette->property(profile).value<QObject*>();
        if (paletteProfile) {
            result = paletteProfile->property(color).value<QColor>();
        }
    }
    return result;
}
/******************************************************************************
 * Divider
 */
UCListItemDivider::UCListItemDivider(QObject *parent)
    : QObject(parent)
    , m_visible(true)
    , m_leftMarginChanged(false)
    , m_rightMarginChanged(false)
    , m_colorFromChanged(false)
    , m_colorToChanged(false)
    , m_thickness(0)
    , m_leftMargin(0)
    , m_rightMargin(0)
    , m_listItem(0)
{
    connect(&UCUnits::instance(), &UCUnits::gridUnitChanged, this, &UCListItemDivider::unitsChanged);
    connect(&UCTheme::instance(), &UCTheme::paletteChanged, this, &UCListItemDivider::paletteChanged);
    unitsChanged();
    paletteChanged();
}
UCListItemDivider::~UCListItemDivider()
{
}

void UCListItemDivider::init(UCListItem *listItem)
{
    QQml_setParent_noEvent(this, listItem);
    m_listItem = UCListItemPrivate::get(listItem);
}

void UCListItemDivider::unitsChanged()
{
    m_thickness = UCUnits::instance().dp(2);
    if (!m_leftMarginChanged) {
        m_leftMargin = UCUnits::instance().dp(2);
    }
    if (!m_rightMarginChanged) {
        m_rightMargin = UCUnits::instance().dp(2);
    }
    if (m_listItem) {
        m_listItem->update();
    }
}

void UCListItemDivider::paletteChanged()
{
    QColor background = getPaletteColor("normal", "background");
    if (!background.isValid()) {
        return;
    }
    // FIXME: we need a palette value for divider colors, till then base on the background
    // luminance
    if (!m_colorFromChanged || !m_colorToChanged) {
        qreal luminance = (background.red()*212 + background.green()*715 + background.blue()*73)/1000.0/255.0;
        bool lightBackground = (luminance > 0.85);
        if (!m_colorFromChanged) {
            m_colorFrom = lightBackground ? QColor("#26000000") : QColor("#26FFFFFF");
        }
        if (!m_colorToChanged) {
            m_colorTo = lightBackground ? QColor("#14FFFFFF") : QColor("#14000000");
        }
        updateGradient();
    }
}

void UCListItemDivider::updateGradient()
{
    m_gradient.clear();
    m_gradient.append(QGradientStop(0.0, m_colorFrom));
    m_gradient.append(QGradientStop(0.49, m_colorFrom));
    m_gradient.append(QGradientStop(0.5, m_colorTo));
    m_gradient.append(QGradientStop(1.0, m_colorTo));
    if (m_listItem) {
        m_listItem->update();
    }
}

QSGNode *UCListItemDivider::paint(QSGNode *node, const QRectF &rect)
{
    QSGRectangleNode *dividerNode = static_cast<QSGRectangleNode*>(node);
    bool lastItem = m_listItem->countOwner ? (m_listItem->index() == (m_listItem->countOwner->property("count").toInt() - 1)): false;
    if (m_visible && !lastItem && (m_gradient.size() > 0) && ((m_colorFrom.alphaF() >= (1.0f / 255.0f)) || (m_colorTo.alphaF() >= (1.0f / 255.0f)))) {
        if (!dividerNode) {
            dividerNode = m_listItem->sceneGraphContext()->createRectangleNode();
        }
        QRectF divider(m_leftMargin, rect.height() - m_thickness, rect.width() - m_leftMargin - m_rightMargin, m_thickness);
        dividerNode->setRect(divider);
        dividerNode->setGradientStops(m_gradient);
        dividerNode->update();
        return dividerNode;
    } else if (node) {
        // delete the node
        delete node;
    }
    return 0;
}

void UCListItemDivider::setVisible(bool visible)
{
    if (m_visible == visible) {
        return;
    }
    m_visible = visible;
    m_listItem->resize();
    m_listItem->update();
    Q_EMIT visibleChanged();
}

void UCListItemDivider::setLeftMargin(qreal leftMargin)
{
    if (m_leftMargin == leftMargin) {
        return;
    }
    m_leftMargin = leftMargin;
    m_leftMarginChanged = true;
    m_listItem->update();
    Q_EMIT leftMarginChanged();
}

void UCListItemDivider::setRightMargin(qreal rightMargin)
{
    if (m_rightMargin == rightMargin) {
        return;
    }
    m_rightMargin = rightMargin;
    m_rightMarginChanged = true;
    m_listItem->update();
    Q_EMIT rightMarginChanged();
}

void UCListItemDivider::setColorFrom(const QColor &color)
{
    if (m_colorFrom == color) {
        return;
    }
    m_colorFrom = color;
    m_colorFromChanged = true;
    updateGradient();
    Q_EMIT colorFromChanged();
}

void UCListItemDivider::setColorTo(const QColor &color)
{
    if (m_colorTo == color) {
        return;
    }
    m_colorTo = color;
    m_colorToChanged = true;
    updateGradient();
    Q_EMIT colorToChanged();
}

/******************************************************************************
 * FlickableControl
 */
FlickableControl::FlickableControl(QObject *parent)
    : QObject(parent)
    , item(qobject_cast<UCListItem*>(parent))
{
}
FlickableControl::~FlickableControl()
{
    grab(false);
    listenToRebind(false);
}

// collect Flickables from ascendants and listens for moves so list item can be rebound
void FlickableControl::listenToRebind(bool listen)
{
    Q_ASSERT(item);
    if (listen) {
        // we should not have any flickables connected
        Q_ASSERT(!list.count());
        // collect flickables and connect their movement
        QQuickItem *parent = QQuickItemPrivate::get(item)->parentItem;
        while (parent) {
            QQuickFlickable *flickable = qobject_cast<QQuickFlickable*>(parent);
            if (flickable) {
                QObject::connect(flickable, SIGNAL(movementStarted()), this, SLOT(rebind()), Qt::DirectConnection);
                // add flickable to the list
                Record record;
                record.flickable = flickable;
                record.interactive = new PropertyChange(flickable, "interactive");
                list.append(record);
            }
            parent = QQuickItemPrivate::get(parent)->parentItem;
        }
    } else {
        Q_FOREACH(const Record &record, list) {
            if (record.flickable) {
                QObject::disconnect(record.flickable.data(), SIGNAL(movementStarted()), this, SLOT(rebind()));
            }
            delete record.interactive;
        }
        // clear the list
        list.clear();
    }
}

void FlickableControl::grab(bool grab)
{
    // go thru the list and block/unblock interactive
    Q_FOREACH(const Record &record, list) {
        if (grab) {
            PropertyChange::setValue(record.interactive, false);
        } else {
            PropertyChange::restore(record.interactive);
        }
    }
}

// reports if any ascendant Flickable is moving
bool FlickableControl::isMoving()
{
    QQuickItem *parent = QQuickItemPrivate::get(item)->parentItem;
    while (parent) {
        QQuickFlickable *flickable = qobject_cast<QQuickFlickable*>(parent);
        if (flickable && flickable->isMoving()) {
            return true;
        }
        parent = QQuickItemPrivate::get(parent)->parentItem;
    }
    return false;
}

// slot to rebind the ListItem
void FlickableControl::rebind()
{
    if (item->contentItem()->x() != 0.0) {
        // content is not in origin, rebind
        UCListItemPrivate::get(item)->_q_rebound();
    } else {
        // we simply do cleanup
        UCListItemPrivate::get(item)->promptRebound();
    }
}

/******************************************************************************
 * ListItemPrivate
 */
UCListItemPrivate::UCListItemPrivate()
    : UCStyledItemBasePrivate()
    , pressed(false)
    , highlightColorChanged(false)
    , tugged(false)
    , ready(false)
    , contentMoving(false)
    , selectable(false)
    , selected(false)
    , xAxisMoveThresholdGU(1.5)
    , overshootGU(2)
    , color(Qt::transparent)
    , highlightColor(Qt::transparent)
    , flickableControl(0)
    , flickable(0)
    , reboundAnimation(0)
    , contentItem(new QQuickItem)
    , disabledOpacity(0)
    , divider(new UCListItemDivider)
    , leadingActions(0)
    , trailingActions(0)
    , selectionPanel(0)
{
}
UCListItemPrivate::~UCListItemPrivate()
{
    delete disabledOpacity;
}

void UCListItemPrivate::init()
{
    Q_Q(UCListItem);
    contentItem->setObjectName("ListItemHolder");
    QQml_setParent_noEvent(contentItem, q);
    contentItem->setParentItem(q);
    divider->init(q);
    // content will be redirected to the contentItem, therefore we must report
    // children changes as it would come from the main component
    QObject::connect(contentItem, &QQuickItem::childrenChanged,
                     q, &UCListItem::childrenChanged);
    q->setFlag(QQuickItem::ItemHasContents);
    // turn activeFocusOnPress on
    q->setActiveFocusOnPress(true);

    // create flickable controller
    flickableControl = new FlickableControl(q);

    // catch theme palette changes
    QObject::connect(&UCTheme::instance(), SIGNAL(paletteChanged()), q, SLOT(_q_updateColors()));
    _q_updateColors();

    // watch size change and set implicit size;
    QObject::connect(&UCUnits::instance(), SIGNAL(gridUnitChanged()), q, SLOT(_q_updateSize()));
    _q_updateSize();

    // watch enabledChanged()
    QObject::connect(q, SIGNAL(enabledChanged()), q, SLOT(_q_dimmDisabled()), Qt::DirectConnection);

    // create rebound animation
    UCUbuntuAnimation animationCodes;
    reboundAnimation = new QQuickPropertyAnimation(q);
    QEasingCurve easing(QEasingCurve::OutElastic);
    easing.setPeriod(0.5);
    reboundAnimation->setEasing(easing);
    reboundAnimation->setDuration(animationCodes.BriskDuration());
    reboundAnimation->setTargetObject(contentItem);
    reboundAnimation->setProperty("x");
    reboundAnimation->setAlwaysRunToEnd(true);
}

// inspired from IS_SIGNAL_CONNECTED(q, UCListItem, pressAndHold, ())
// the macro cannot be used due to Arguments cannot be an empty ()
bool UCListItemPrivate::isPressAndHoldConnected()
{
    Q_Q(UCListItem);
    void (UCListItem::*signal)() = &UCListItem::pressAndHold;
    static QMetaMethod method = QMetaMethod::fromSignal(signal);
    static int signalIdx = QMetaObjectPrivate::signalIndex(method);
    return QObjectPrivate::get(q)->isSignalConnected(signalIdx);
}

void UCListItemPrivate::_q_updateColors()
{
    Q_Q(UCListItem);
    highlightColor = getPaletteColor("selected", "background");
    q->update();
}

void UCListItemPrivate::_q_dimmDisabled()
{
    Q_Q(UCListItem);
    if (q->isEnabled()) {
        PropertyChange::restore(disabledOpacity);
    } else if (opacity() != 0.5) {
        // this is the first time we need to create the property change
        if (!disabledOpacity) {
            disabledOpacity = new PropertyChange(q, "opacity");
        }
        PropertyChange::setValue(disabledOpacity, 0.5);
    }
}

void UCListItemPrivate::_q_rebound()
{
    setPressed(false);
    // initiate rebinding only if there were actions tugged
    Q_Q(UCListItem);
    if (!UCListItemActionsPrivate::isConnectedTo(leadingActions, q) && !UCListItemActionsPrivate::isConnectedTo(trailingActions, q)) {
        return;
    }
    setTugged(false);
    // connect rebound completion so we can disconnect the action lists
    // then rebound to zero
    snapTo(0);
}
void UCListItemPrivate::_q_completeRebinding()
{
    // disconnect animation, otherwise snapping will disconnect the panel
    QObject::disconnect(reboundAnimation, 0, 0, 0);
    // restore flickable's interactive and cleanup
    flickableControl->grab(false);
    // no need to listen flickables any longer
    flickableControl->listenToRebind(false);
    // disconnect actions
    grabPanel(leadingActions, false);
    grabPanel(trailingActions, false);
    // set contentMoved to false
    setContentMoved(false);
}
void UCListItemPrivate::_q_completeSnapping()
{
    QObject::disconnect(reboundAnimation, 0, 0, 0);
    setContentMoved(false);
}

void UCListItemPrivate::_q_updateIndex()
{
    Q_Q(UCListItem);
    q->update();
}

// returns the index of the list item when used in model driven views,
// and the child index in other cases
int UCListItemPrivate::index()
{
    Q_Q(UCListItem);
    // is there an index context property?
    QQmlContext *context = qmlContext(q);
    QVariant index = context->contextProperty("index");
    return index.isValid() ?
                index.toInt() :
                (parentItem ? QQuickItemPrivate::get(parentItem)->childItems.indexOf(q) : -1);
}

/*!
 * \qmlproperty bool ListItem::moved
 * The property signals the move of the list item's content. It is set whenever
 * the content is tugged and reset when the snapping and rebounding animations
 * complete.
 *
 * \sa movingStarted, movingEnded
 */

/*!
 * \qmlsignal ListItem::movingStarted
 * Signal emitted when the moving of the list item content is started.
 */
/*!
 * \qmlsignal ListItem::movingEnded
 * Signal emitted when the moving of the list item content is ended.
 */
bool UCListItemPrivate::isMoving() const
{
    return contentMoving;
}
// the function drives the moving property
void UCListItemPrivate::setContentMoved(bool move)
{
    if (contentMoving == move) {
        return;
    }
    contentMoving = move;
    Q_Q(UCListItem);
    if (move) {
        Q_EMIT q->movingStarted();
    } else {
        Q_EMIT q->movingEnded();
    }
    Q_EMIT q->movingChanged();
}

void UCListItemPrivate::_q_updateSelected()
{
    Q_Q(UCListItem);
    bool checked = selectionPanel->property("checked").toBool();
    q->setSelected(checked);
    update();
}

// the function performs a prompt rebound on mouse release without any animation
void UCListItemPrivate::promptRebound()
{
    setPressed(false);
    setTugged(false);
    _q_completeRebinding();
}
// rebounds or snaps to a given x position
void UCListItemPrivate::snapTo(qreal x)
{
    // if the value given is 0.0, we snap out (rebound), otherwise we snap in
    if (x != 0.0) {
        // snap
        QObject::connect(reboundAnimation, SIGNAL(stopped()), q_ptr, SLOT(_q_completeSnapping()));
    } else {
        QObject::connect(reboundAnimation, SIGNAL(stopped()), q_ptr, SLOT(_q_completeRebinding()));
    }
    reboundAnimation->setFrom(contentItem->x());
    reboundAnimation->setTo(x);
    reboundAnimation->restart();
    setContentMoved(true);
}

// set pressed flag and update background
// called when units size changes
void UCListItemPrivate::_q_updateSize()
{
    Q_Q(UCListItem);
    QQuickItem *owner = flickable ? flickable : parentItem;
    q->setImplicitWidth(owner ? owner->width() : UCUnits::instance().gu(40));
    q->setImplicitHeight(UCUnits::instance().gu(7));
}

// returns true if the click happened over an inactive component
bool UCListItemPrivate::canHighlight(QMouseEvent *event)
{
    // localPos is a position relative to ListItem which will give us a child from
    // from the original coordinates; therefore we must map the position to the contentItem
    Q_Q(UCListItem);
    QPointF myPos = q->mapToItem(contentItem, event->localPos());
    QQuickItem *child = contentItem->childAt(myPos.x(), myPos.y());
    return !child || qobject_cast<QQuickText*>(child) ||
           ((child->acceptedMouseButtons() & event->button()) != event->button());
}

// set pressed flag and update contentItem
void UCListItemPrivate::setPressed(bool pressed)
{
    if (this->pressed != pressed) {
        this->pressed = pressed;
        Q_Q(UCListItem);
        q->update();
        Q_EMIT q->pressedChanged();
    }
}
// toggles the tugged flag and installs/removes event filter
void UCListItemPrivate::setTugged(bool tugged)
{
    suppressClick = tugged;
    if (this->tugged == tugged) {
        return;
    }
    this->tugged = tugged;
    Q_Q(UCListItem);
    QQuickWindow *window = q->window();
    if (tugged) {
        window->installEventFilter(q);
    } else {
        window->removeEventFilter(q);
    }
}

// sets the tugged flag but also grabs the panels from the leading/trailing actions
bool UCListItemPrivate::grabPanel(UCListItemActions *actionsList, bool isTugged)
{
    Q_Q(UCListItem);
    if (isTugged) {
        bool grab = UCListItemActionsPrivate::connectToListItem(actionsList, q, (actionsList == leadingActions));
        if (actionsList) {
            flickableControl->grab(grab);
        }
        return grab;
    } else {
        UCListItemActionsPrivate::disconnectFromListItem(actionsList);
        return false;
    }
}

void UCListItemPrivate::resize()
{
    Q_Q(UCListItem);
    QRectF rect(q->boundingRect());
    if (divider && divider->m_visible) {
        rect.setHeight(rect.height() - divider->m_thickness);
    }
    contentItem->setSize(rect.size());
}

void UCListItemPrivate::update()
{
    if (!ready) {
        return;
    }
    Q_Q(UCListItem);
    q->update();
}

void UCListItemPrivate::clampX(qreal &x, qreal dx)
{
    UCListItemActionsPrivate *leading = UCListItemActionsPrivate::get(leadingActions);
    UCListItemActionsPrivate *trailing = UCListItemActionsPrivate::get(trailingActions);
    x += dx;
    // min cannot be less than the trailing's panel width
    qreal min = (trailing && trailing->panelItem) ? -trailing->panelItem->width() - UCUnits::instance().gu(overshootGU): 0;
    // max cannot be bigger than 0 or the leading's width in case we have leading panel
    qreal max = (leading && leading->panelItem) ? leading->panelItem->width() + UCUnits::instance().gu(overshootGU): 0;
    x = CLAMP(x, min, max);
}

QQuickItem *UCListItemPrivate::createSelectionPanel()
{
    Q_Q(UCListItem);
    if (!selectionPanel) {
        QUrl panelDocument = UbuntuComponentsPlugin::pluginUrl().
                resolved(QUrl::fromLocalFile("ListItemSelectablePanel.qml"));
        QQmlComponent component(qmlEngine(q), panelDocument);
        if (!component.isError()) {
            selectionPanel = qobject_cast<QQuickItem*>(component.beginCreate(qmlContext(q)));
            if (selectionPanel) {
                QQml_setParent_noEvent(selectionPanel, q);
                selectionPanel->setParentItem(q);
                selectionPanel->setVisible(false);
                selectionPanel->setProperty("checked", selected);
                // complete component creation
                component.completeCreate();
            }
        } else {
            qmlInfo(q) << component.errorString();
        }
    }
    return selectionPanel;
}
void UCListItemPrivate::toggleSelectionMode()
{
    if (!createSelectionPanel()) {
        return;
    }
    Q_Q(UCListItem);
    if (selectable) {
        // move and dimm content item
        selectionPanel->setVisible(true);
        snapTo(selectionPanel->width());
        // sync selected flag with the attached selection array
        if (attachedObject) {
            q->setSelected(attachedObject->isItemSelected(q));
        }
        QObject::connect(selectionPanel, SIGNAL(checkedChanged()), q, SLOT(_q_updateSelected()));
    } else {
        // remove content item dimming and destroy selection panel as well
        snapTo(0.0);
        selectionPanel->setVisible(false);
        QObject::disconnect(selectionPanel, SIGNAL(checkedChanged()), q, SLOT(_q_updateSelected()));
    }
    _q_updateSelected();
}

/*!
 * \qmltype ListItem
 * \instantiates UCListItem
 * \inqmlmodule Ubuntu.Components 1.2
 * \ingroup unstable-ubuntu-listitems
 * \since Ubuntu.Components 1.2
 * \brief The ListItem element provides Ubuntu design standards for list or grid
 * views.
 *
 * The component is dedicated to be used in designs with static or dynamic lists
 * (i.e. list views where each item's layout differs or in lists where the content
 * is determined by a given model, thus each element has the same layout). The
 * element does not define any specific layout, components can be placed in any
 * ways on it. However, when used in list views, the content must be carefully
 * chosen to in order to keep the kinetic behavior and the highest FPS possible.
 *
 * The component provides two color properties which configures the item's background
 * when normal or pressed. This can be configures through \l color and \l highlightColor
 * properties.
 *
 * \c contentItem holds all components and resources declared as child to ListItem.
 * Being an Item, all other properties can be accessed or altered, with the exception
 * of some:
 * \list A
 * \li do not alter \c x, \c y, \c width or \c height properties as those are
 *      controlled by the ListItem itself when leading or trailing actions are
 *      revealed and thus will destroy your logic
 * \li never anchor left or right anchor lines as it will block revealing the actions.
 * \endlist
 *
 * Each ListItem has a thin divider shown on the bottom of the component. This
 * divider can be configured through the \l divider grouped property, which can
 * configure its margins from the edges of the ListItem as well as its visibility.
 * When used in \c ListView or \l UbuntuListView, the last list item will not
 * show the divider no matter of the visible property value set.
 *
 * ListItem can handle actions that can get tugged from front to back of the item.
 * These actions are Action elements visualized in panels attached to the front
 * or to the back of the item, and are revealed by swiping the item horizontally.
 * The tug is started only after the mouse/touch move had passed a given threshold.
 * These actions are configured through the \l leadingActions as well as \l
 * trailingActions properties.
 * \qml
 * ListItem {
 *     id: listItem
 *     leadingActions: ListItemActions {
 *         actions: [
 *             Action {
 *                 iconName: "delete"
 *                 onTriggered: listItem.destroy()
 *             }
 *         ]
 *     }
 *     trailingActions: ListItemActions {
 *         actions: [
 *             Action {
 *                 iconName: "search"
 *                 onTriggered: {
 *                     // do some search
 *                 }
 *             }
 *         ]
 *     }
 * }
 * \endqml
 * \note When a list item is tugged, it automatically connects both leading and
 * trailing actions to the list item. This implies that a ListItem cannot use
 * the same ListItemActions instance for both leading and trailing actions. If
 * it is desired to have the same action present in both leading and trailing
 * actions, one of the ListItemActions actions list can use the other's list. In
 * the following example the list item can be deleted through both leading and
 * trailing actions:
 * \qml
 * ListItem {
 *     id: listItem
 *     leadingActions: ListItemActions {
 *         actions: [
 *             Action {
 *                 iconName: "delete"
 *                 onTriggered: listItem.destroy()
 *             }
 *         ]
 *     }
 *     trailingActions: ListItemActions {
 *         actions: leadingActions.actions
 *     }
 * }
 * \endqml
 * \sa ListItemActions
 */

/*!
 * \qmlsignal ListItem::clicked()
 * The signal is emitted when the component gets released while the \l pressed property
 * is set. The signal is not emitted if the ListItem content is tugged or when used in
 * Flickable (or ListView, GridView) and the Flickable gets moved.
 *
 * If the ListItem contains a component which contains a MouseArea, the clicked
 * signal will be supressed.
 */

/*!
 * \qmlsignal ListItem::pressAndHold()
 * The signal is emitted when the list item is long pressed. When a slot is connected,
 * no \l clicked signal will be emitted, similarly to MouseArea's pressAndHold.
 *
 * If the ListItem contains a component which contains a MouseArea, the pressAndHold
 * signal will be supressed.
 */

UCListItem::UCListItem(QQuickItem *parent)
    : UCStyledItemBase(*(new UCListItemPrivate), parent)
{
    Q_D(UCListItem);
    d->init();
}

UCListItem::~UCListItem()
{
}

void UCListItem::componentComplete()
{
    UCStyledItemBase::componentComplete();
    Q_D(UCListItem);
    d->ready = true;
    /* We only deal with ListView, as for other cases we would need to check the children
     * changes, which would have an enormous impact on performance in case of huge amount
     * of items. However, if the parent item, or Flickable declares a "count" property,
     * the ListItem will take use of it!
     */
    d->countOwner = (d->flickable && d->flickable->property("count").isValid()) ?
                d->flickable :
                (d->parentItem && d->parentItem->property("count").isValid()) ? d->parentItem : 0;
    if (d->countOwner) {
        QObject::connect(d->countOwner.data(), SIGNAL(countChanged()),
                         this, SLOT(_q_updateIndex()), Qt::DirectConnection);
        update();
    }

    // get the selected state from the attached object
    if (d->attachedObject) {
        setSelected(d->attachedObject->isItemSelected(this));
    }
}

void UCListItem::itemChange(ItemChange change, const ItemChangeData &data)
{
    UCStyledItemBase::itemChange(change, data);
    if (change == ItemParentHasChanged) {
        Q_D(UCListItem);
        // make sure we are not connected to any previous Flickable
        d->flickableControl->listenToRebind(false);
        // check if we are in a positioner, and if that positioner is in a Flickable
        QQuickBasePositioner *positioner = qobject_cast<QQuickBasePositioner*>(data.item);
        if (positioner && positioner->parentItem()) {
            // count owner is a positioner
            d->flickable = qobject_cast<QQuickFlickable*>(positioner->parentItem()->parentItem());
        } else if (data.item && data.item->parentItem()){
            // check if we are in a Flickable then
            d->flickable = qobject_cast<QQuickFlickable*>(data.item->parentItem());
        }

        // attach ListItem properties to the flickable or parentItem
        if (d->flickable) {
            d->attachedObject = static_cast<UCListItemAttached*>(
                        qmlAttachedPropertiesObject<UCListItem>(d->flickable));
        } else if (data.item) {
            d->attachedObject = static_cast<UCListItemAttached*>(
                        qmlAttachedPropertiesObject<UCListItem>(data.item));
        }

        if (d->flickable) {
            // connect to flickable to get width changes
            QObject::connect(d->flickable, SIGNAL(widthChanged()), this, SLOT(_q_updateSize()));
        } else if (data.item) {
            QObject::connect(data.item, SIGNAL(widthChanged()), this, SLOT(_q_updateSize()));
        } else {
            // mar as not ready, so no action should be performed which depends on readyness
            d->ready = false;
        }

        // update size
        d->_q_updateSize();
    }
}

void UCListItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    UCStyledItemBase::geometryChanged(newGeometry, oldGeometry);
    // resize contentItem item
    Q_D(UCListItem);
    d->resize();
}

QSGNode *UCListItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    Q_UNUSED(data);

    Q_D(UCListItem);
    QColor color = (d->pressed || (d->selectable && d->selected))? d->highlightColor : d->color;

    if (width() <= 0 || height() <= 0) {
        delete oldNode;
        return 0;
    }

    QSGRectangleNode *rectNode = 0;
    rectNode = static_cast<QSGRectangleNode*>(oldNode);
    if (!rectNode) {
        rectNode = QQuickItemPrivate::get(this)->sceneGraphContext()->createRectangleNode();
    }
    if (color.alphaF() >= (1.0f / 255.0f)) {
        rectNode->setColor(color);
        // cover only the area of the contentItem
        rectNode->setRect(d->contentItem->boundingRect());
        rectNode->setGradientStops(QGradientStops());
        rectNode->setAntialiasing(true);
        rectNode->setAntialiasing(false);
        rectNode->update();
    } else {
        // delete node, this will delete the divider node as well
        delete rectNode;
        rectNode = 0;
    }
    oldNode = rectNode;
    QSGNode *dividerNode = oldNode ? oldNode->childAtIndex(0) : 0;
    if (d->divider && d->divider->m_visible) {
        QSGNode *newNode = d->divider->paint(dividerNode, boundingRect());
        if (newNode != dividerNode && oldNode) {
            if (dividerNode) {
                oldNode->removeChildNode(dividerNode);
            }
            if (newNode) {
                oldNode->appendChildNode(newNode);
            }
        }
        if (!oldNode) {
            oldNode = newNode;
        }
    } else if (dividerNode) {
        // the divider painter node may be still added as child, so remove it
        oldNode->removeChildNode(dividerNode);
    }
    return oldNode;
}

void UCListItem::mousePressEvent(QMouseEvent *event)
{
    UCStyledItemBase::mousePressEvent(event);
    Q_D(UCListItem);
    if (d->selectable || d->flickableControl->isMoving()) {
        // while moving, we cannot select or tug any items
        return;
    }
    if (!d->suppressClick && !d->pressed && event->button() == Qt::LeftButton && d->canHighlight(event)) {
        d->setPressed(true);
        d->lastPos = d->pressedPos = event->localPos();
        // connect the Flickable to know when to rebound
        d->flickableControl->listenToRebind(true);
        // start pressandhold timer
        d->pressAndHoldTimer.start(QGuiApplication::styleHints()->mousePressAndHoldInterval(), this);
        // if it was moved, grab the panels
        if (d->tugged) {
            d->grabPanel(d->leadingActions, true);
            d->grabPanel(d->trailingActions, true);
        }
    }
    // accept the event so we get the rest of the events as well
    event->setAccepted(true);
}

void UCListItem::mouseReleaseEvent(QMouseEvent *event)
{
    UCStyledItemBase::mouseReleaseEvent(event);
    Q_D(UCListItem);
    if (d->selectable) {
        // no move is allowed while selectable mode is on
        return;
    }
    d->pressAndHoldTimer.stop();
    // set released
    if (d->pressed) {
        // disconnect the flickable
        d->flickableControl->listenToRebind(false);

        d->setContentMoved(true);
        if (!d->suppressClick) {
            Q_EMIT clicked();
            d->_q_rebound();
        } else {
            // unset dragging in panel item
            UCListItemActionsPrivate::setDragging(d->leadingActions, this, false);
            UCListItemActionsPrivate::setDragging(d->trailingActions, this, false);

            d->suppressClick = false;
        }
    }
    d->setPressed(false);
}

void UCListItem::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(UCListItem);
    UCStyledItemBase::mouseMoveEvent(event);

    if (d->selectable) {
        // no move is allowed while selectable mode is on
        return;
    }

    // accept the tugging only if the move is within the threshold
    bool leadingAttached = UCListItemActionsPrivate::isConnectedTo(d->leadingActions, this);
    bool trailingAttached = UCListItemActionsPrivate::isConnectedTo(d->trailingActions, this);
    if (d->pressed && !(leadingAttached || trailingAttached)) {
        // check if we can initiate the drag at all
        // only X direction matters, if Y-direction leaves the threshold, but X not, the tug is not valid
        qreal threshold = UCUnits::instance().gu(d->xAxisMoveThresholdGU);
        qreal mouseX = event->localPos().x();
        qreal pressedX = d->pressedPos.x();

        if ((mouseX < (pressedX - threshold)) || (mouseX > (pressedX + threshold))) {
            // the press went out of the threshold area, enable move, if the direction allows it
            d->lastPos = event->localPos();
            // connect both panels
            leadingAttached = d->grabPanel(d->leadingActions, true);
            trailingAttached = d->grabPanel(d->trailingActions, true);
        }
    }

    if (leadingAttached || trailingAttached) {
        qreal x = d->contentItem->x();
        qreal dx = event->localPos().x() - d->lastPos.x();
        d->lastPos = event->localPos();

        if (dx) {
            // stop pressAndHold timer as we started to drag
            d->pressAndHoldTimer.stop();
            d->setContentMoved(true);
            // clamp X into allowed dragging area
            d->clampX(x, dx);
            // block flickable
            d->setTugged(true);
            d->contentItem->setX(x);

            // decide which panel is visible by checking the contentItem's X coordinates
            if (d->contentItem->x() > 0) {
                if (leadingAttached) {
                    UCListItemActionsPrivate::get(d->leadingActions)->panelItem->setVisible(true);
                }
                if (trailingAttached) {
                    UCListItemActionsPrivate::get(d->trailingActions)->panelItem->setVisible(false);
                }
            } else if (d->contentItem->x() < 0) {
                // trailing revealed
                if (leadingAttached) {
                    UCListItemActionsPrivate::get(d->leadingActions)->panelItem->setVisible(false);
                }
                if (trailingAttached) {
                    UCListItemActionsPrivate::get(d->trailingActions)->panelItem->setVisible(true);
                }
            }

            // set dragging in panel item
            UCListItemActionsPrivate::setDragging(d->leadingActions, this, true);
            UCListItemActionsPrivate::setDragging(d->trailingActions, this, true);
        }
    }
}

bool UCListItem::childMouseEventFilter(QQuickItem *child, QEvent *event)
{
    QEvent::Type type = event->type();
    if (type == QEvent::MouseButtonPress) {
        // suppress click event if pressed over an active area, except Text, which can also handle
        // mouse clicks when content is an URL
        QMouseEvent *mouse = static_cast<QMouseEvent*>(event);
        if (child->isEnabled() && child->acceptedMouseButtons() & mouse->button() && !qobject_cast<QQuickText*>(child)) {
            Q_D(UCListItem);
            d->suppressClick = true;
        }
    } else if (type == QEvent::MouseButtonRelease) {
        Q_D(UCListItem);
        d->suppressClick = false;
    }
    return UCStyledItemBase::childMouseEventFilter(child, event);
}

bool UCListItem::eventFilter(QObject *target, QEvent *event)
{
    QPointF myPos;
    // only filter press events, and rebound when pressed outside
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouse = static_cast<QMouseEvent*>(event);
        QQuickWindow *window = qobject_cast<QQuickWindow*>(target);
        if (window) {
            myPos = window->contentItem()->mapToItem(this, mouse->localPos());
        }
    } else if (event->type() == QEvent::TouchBegin) {
        QTouchEvent *touch = static_cast<QTouchEvent*>(event);
        QQuickWindow *window = qobject_cast<QQuickWindow*>(target);
        if (window) {
            myPos = window->contentItem()->mapToItem(this, touch->touchPoints()[0].pos());
        }
    }
    if (!myPos.isNull() && !contains(myPos)) {
        Q_D(UCListItem);
        d->_q_rebound();
        // only accept event, but let it be handled by the underlying or surrounding Flickables
        event->accept();
    }
    return UCStyledItemBase::eventFilter(target, event);
}

void UCListItem::timerEvent(QTimerEvent *event)
{
    Q_D(UCListItem);
    if (event->timerId() == d->pressAndHoldTimer.timerId()) {
        d->pressAndHoldTimer.stop();
        if (isEnabled() && d->isPressAndHoldConnected()) {
            d->suppressClick = true;
            Q_EMIT pressAndHold();
        }
    } else {
        QQuickItem::timerEvent(event);
    }
}

/*!
 * \qmlproperty ListItemActions ListItem::leadingActions
 *
 * The property holds the actions and its configuration to be revealed when swiped
 * from left to right.
 *
 * \sa trailingActions
 */
UCListItemActions *UCListItem::leadingActions() const
{
    Q_D(const UCListItem);
    return d->leadingActions;
}
void UCListItem::setLeadingActions(UCListItemActions *actions)
{
    Q_D(UCListItem);
    if (d->leadingActions == actions) {
        return;
    }
    // snap out before we change the actions
    d->promptRebound();
    // then delete panelItem
    if (d->leadingActions) {
        UCListItemActionsPrivate *list = UCListItemActionsPrivate::get(d->leadingActions);
        delete list->panelItem;
        list->panelItem = 0;
    }
    d->leadingActions = actions;
    if (d->leadingActions == d->trailingActions && d->leadingActions) {
        qmlInfo(this) << UbuntuI18n::tr("leadingActions and trailingActions cannot share the same object!");
    }
    Q_EMIT leadingActionsChanged();
}

/*!
 * \qmlproperty ListItemActions ListItem::trailingActions
 *
 * The property holds the actions and its configuration to be revealed when swiped
 * from right to left.
 *
 * \sa leadingActions
 */
UCListItemActions *UCListItem::trailingActions() const
{
    Q_D(const UCListItem);
    return d->trailingActions;
}
void UCListItem::setTrailingActions(UCListItemActions *actions)
{
    Q_D(UCListItem);
    if (d->trailingActions == actions) {
        return;
    }
    // snap out before we change the actions
    d->promptRebound();
    // then delete panelItem
    if (d->trailingActions) {
        UCListItemActionsPrivate *list = UCListItemActionsPrivate::get(d->trailingActions);
        delete list->panelItem;
        list->panelItem = 0;
    }
    d->trailingActions = actions;
    if (d->leadingActions == d->trailingActions && d->trailingActions) {
        qmlInfo(this) << UbuntuI18n::tr("leadingActions and trailingActions cannot share the same object!");
    }
    Q_EMIT trailingActionsChanged();
}

/*!
 * \qmlproperty Item ListItem::contentItem
 *
 * contentItem holds the components placed on a ListItem.
 */
QQuickItem* UCListItem::contentItem() const
{
    Q_D(const UCListItem);
    return d->contentItem;
}

/*!
 * \qmlpropertygroup ::ListItem::divider
 * \qmlproperty bool ListItem::divider.visible
 * \qmlproperty real ListItem::divider.leftMargin
 * \qmlproperty real ListItem::divider.rightMargin
 * \qmlproperty real ListItem::divider.colorFrom
 * \qmlproperty real ListItem::divider.colorTo
 *
 * This grouped property configures the thin divider shown in the bottom of the
 * component. Configures the visibility and the margins from the left and right
 * of the ListItem. When tugged (swiped left or right to reveal the actions),
 * it is not moved together with the content. \c colorFrom and \c colorTo configure
 * the starting and ending colors of the divider.
 *
 * When \c visible is true, the ListItem's content size gets thinner with the
 * divider's \c thickness.
 *
 * The default values for the properties are:
 * \list
 * \li \c visible: true
 * \li \c leftMargin: 2 GU
 * \li \c rightMargin: 2 GU
 * \endlist
 */
UCListItemDivider* UCListItem::divider() const
{
    Q_D(const UCListItem);
    return d->divider;
}

/*!
 * \qmlproperty bool ListItem::pressed
 * True when the item is pressed. The items stays pressed when the mouse or touch
 * is moved horizontally. When in Flickable (or ListView), the item gets un-pressed
 * (false) when the mouse or touch is moved towards the vertical direction causing
 * the flickable to move.
 */
bool UCListItem::pressed() const
{
    Q_D(const UCListItem);
    return d->pressed;
}

/*!
 * \qmlproperty color ListItem::color
 * Configures the color of the normal background. The default value is transparent.
 */
QColor UCListItem::color() const
{
    Q_D(const UCListItem);
    return d->color;
}
void UCListItem::setColor(const QColor &color)
{
    Q_D(UCListItem);
    if (d->color == color) {
        return;
    }
    d->color = color;
    update();
    Q_EMIT colorChanged();
}

/*!
 * \qmlproperty color ListItem::highlightColor
 * Configures the color when pressed. Defaults to the theme palette's background color.
 */
QColor UCListItem::highlightColor() const
{
    Q_D(const UCListItem);
    return d->highlightColor;
}
void UCListItem::setHighlightColor(const QColor &color)
{
    Q_D(UCListItem);
    if (d->highlightColor == color) {
        return;
    }
    d->highlightColor = color;
    // no more theme change watch
    disconnect(&UCTheme::instance(), SIGNAL(paletteChanged()), this, SLOT(_q_updateColors()));
    update();
    Q_EMIT highlightColorChanged();
}

/*!
 * \qmlproperty bool ListItem::selectable
 * The property drives whether a list item is selectable or not. When set, the item
 * will show a check box on the leading side hanving the content item pushed towards
 * trailing side and dimmed. The checkbox which will reflect and drive the \l selected
 * state.
 * Defaults to false.
 */
bool UCListItem::selectable() const
{
    Q_D(const UCListItem);
    return d->selectable;
}
void UCListItem::setSelectable(bool selectable)
{
    Q_D(UCListItem);
    if (d->selectable == selectable) {
        return;
    }
    d->selectable = selectable;
    d->toggleSelectionMode();
    Q_EMIT selectableChanged();
}

/*!
 * \qmlproperty bool ListItem::selected
 * The property drives whether a list item is selected or not. While selected, the
 * ListItem is dimmed and cannot be tugged. The default value is false.
 */
bool UCListItem::selected() const
{
    Q_D(const UCListItem);
    return d->selected;
}
void UCListItem::setSelected(bool selected)
{
    Q_D(UCListItem);
    if (d->selected == selected) {
        return;
    }
    d->selected = selected;
    // update attached list
    if (d->attachedObject) {
        if (selected) {
            d->attachedObject->addSelectedItem(this);
        } else {
            d->attachedObject->removeSelectedItem(this);
        }
    }
    // update panel as well
    if (d->selectionPanel) {
        d->selectionPanel->setProperty("checked", d->selected);
    }
    Q_EMIT selectedChanged();
}

/*!
 * \qmlproperty list<Object> ListItem::data
 * \default
 * Overloaded default property containing all the children and resources.
 */
QQmlListProperty<QObject> UCListItem::data()
{
    Q_D(UCListItem);
    return QQuickItemPrivate::get(d->contentItem)->data();
}

/*!
 * \qmlproperty list<Item> ListItem::children
 * Overloaded default property containing all the visible children of the item.
 */
QQmlListProperty<QQuickItem> UCListItem::children()
{
    Q_D(UCListItem);
    return QQuickItemPrivate::get(d->contentItem)->children();
}

/******************************************************************************
 * ListItem attached
 */
UCListItemAttached *UCListItem::qmlAttachedProperties(QObject *owner)
{
    return new UCListItemAttached(owner);
}

UCListItemAttached::UCListItemAttached(QObject *parent)
    : QObject(parent)
{
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
    return m_indexes;
}
void UCListItemAttached::setSelectedIndexes(const QList<int> &list)
{
    if (m_indexes == list) {
        return;
    }
    m_indexes = list;
    Q_EMIT selectedIndexesChanged();
}

void UCListItemAttached::addSelectedItem(UCListItem *item)
{
    int index = UCListItemPrivate::get(item)->index();
    if (!m_indexes.contains(index)) {
        m_indexes.append(index);
        Q_EMIT selectedIndexesChanged();
    }
}
void UCListItemAttached::removeSelectedItem(UCListItem *item)
{
    m_indexes.removeAll(UCListItemPrivate::get(item)->index());
}

bool UCListItemAttached::isItemSelected(UCListItem *item)
{
    return m_indexes.contains(UCListItemPrivate::get(item)->index());
}

#include "moc_uclistitem.cpp"
