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
#include "ucaction.h"
#include <QtQml/QQmlInfo>
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickflickable_p.h>
#include <QtQuick/private/qquickpositioners_p.h>
#include <QtQuick/private/qquickmousearea_p.h>
#include <QtGui/QGuiApplication>
#include <QtGui/QStyleHints>
#include <QtQuick/private/qquickanimation_p.h>
#include "uclistitemstyle.h"

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
 * SnapAnimator
 */
UCListItemSnapAnimator::UCListItemSnapAnimator(UCListItem *item)
    : QObject(item)
    , item(item)
{
}
UCListItemSnapAnimator::~UCListItemSnapAnimator()
{
    // make sure we cannot animate anymore, for safety
    item = 0;
}

bool UCListItemSnapAnimator::snap(qreal to)
{
    if (!item) {
        return false;
    }
    UCListItemPrivate *listItem = UCListItemPrivate::get(item);
    QQuickPropertyAnimation *snap = getDefaultAnimation();
    if (!snap) {
        // no animation, so we simply position the component
        listItem->contentItem->setX(to);
        return false;
    }
    snap->setTargetObject(listItem->contentItem);
    if (to == 0.0) {
        QObject::connect(snap, &QQuickAbstractAnimation::stopped,
                         this, &UCListItemSnapAnimator::snapOut);
    } else {
        QObject::connect(snap, &QQuickAbstractAnimation::stopped,
                         this, &UCListItemSnapAnimator::snapIn);
    }
    if (snap->properties().isEmpty() && snap->property().isEmpty()) {
        snap->setProperty("x");
    }
    snap->setFrom(listItem->contentItem->property(snap->property().toLocal8Bit().constData()));
    snap->setTo(to);
    snap->setAlwaysRunToEnd(true);
    listItem->setContentMoving(true);
    snap->start();
    return true;
}

void UCListItemSnapAnimator::complete()
{
    QQuickPropertyAnimation *snap = getDefaultAnimation();
    if (snap && snap->isRunning()) {
        snap->complete();
    }
}

void UCListItemSnapAnimator::snapOut()
{
    if (senderSignalIndex() >= 0) {
        // disconnect animation, otherwise snapping will disconnect the panel
        QQuickAbstractAnimation *snap = getDefaultAnimation();
        QObject::disconnect(snap, 0, 0, 0);
    }
    UCListItemPrivate *listItem = UCListItemPrivate::get(item);
    if (listItem->attachedProperties) {
        // restore flickable's interactive and cleanup
        listItem->attachedProperties->disableInteractive(item, false);
        // no need to listen flickables any longer
        listItem->attachedProperties->listenToRebind(item, false);
    }
    // disconnect actions
    listItem->grabPanel(listItem->leadingActions, false);
    listItem->grabPanel(listItem->trailingActions, false);
    // set contentMoved to false
    listItem->setContentMoving(false);
}

void UCListItemSnapAnimator::snapIn()
{
    if (senderSignalIndex() >= 0) {
        // disconnect animation
        QQuickAbstractAnimation *snap = getDefaultAnimation();
        QObject::disconnect(snap, 0, 0, 0);
    }
    UCListItemPrivate *listItem = UCListItemPrivate::get(item);
    listItem->setContentMoving(false);
}

QQuickPropertyAnimation *UCListItemSnapAnimator::getDefaultAnimation()
{
    UCListItemPrivate *listItem = UCListItemPrivate::get(item);
    if (!listItem->styleItem && listItem->loadStyle()) {
        listItem->initStyleItem();
    }
    return listItem->styleItem ? listItem->styleItem->m_snapAnimation : 0;
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
 * ListItemPrivate
 */
UCListItemPrivate::UCListItemPrivate()
    : UCStyledItemBasePrivate()
    , pressed(false)
    , contentMoved(false)
    , highlightColorChanged(false)
    , tugged(false)
    , suppressClick(false)
    , ready(false)
    , selectable(false)
    , selected(false)
    , customStyle(false)
    , customColor(false)
    , highlight(UCListItem::AutomaticHighlight)
    , xAxisMoveThresholdGU(1.5)
    , overshoot(UCUnits::instance().gu(2))
    , color(Qt::transparent)
    , highlightColor(Qt::transparent)
    , flickable(0)
    , attachedProperties(0)
    , contentItem(new QQuickItem)
    , disabledOpacity(0)
    , divider(new UCListItemDivider)
    , leadingActions(0)
    , trailingActions(0)
    , selectionPanel(0)
    , animator(0)
    , action(0)
    , styleComponent(0)
    , styleItem(0)
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
                     q, &UCListItem::listItemChildrenChanged);
    q->setFlag(QQuickItem::ItemHasContents);
    // turn activeFocusOnPress on
    q->setActiveFocusOnPress(true);

    // catch theme changes
    QObject::connect(&UCTheme::instance(), SIGNAL(nameChanged()), q, SLOT(_q_updateThemedData()));
    _q_updateThemedData();

    // watch size change and set implicit size;
    QObject::connect(&UCUnits::instance(), SIGNAL(gridUnitChanged()), q, SLOT(_q_updateSize()));
    _q_updateSize();

    // watch enabledChanged()
    QObject::connect(q, SIGNAL(enabledChanged()), q, SLOT(_q_dimmDisabled()), Qt::DirectConnection);

    // create the animator
    animator = new UCListItemSnapAnimator(q);
}

// inspired from IS_SIGNAL_CONNECTED(q, UCListItem, pressAndHold, ())
// the macro cannot be used due to Arguments cannot be an empty ()
bool UCListItemPrivate::isPressAndHoldConnected()
{
    Q_Q(UCListItem);
    static QMetaMethod method = QMetaMethod::fromSignal(&UCListItem::pressAndHold);
    static int signalIdx = QMetaObjectPrivate::signalIndex(method);
    return QObjectPrivate::get(q)->isSignalConnected(signalIdx);
}

void UCListItemPrivate::_q_updateThemedData()
{
    // update colors, panels
    if (!customColor) {
        Q_Q(UCListItem);
        highlightColor = getPaletteColor("selected", "background");
        q->update();
    }
    loadStyle();
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

void UCListItemPrivate::_q_updateSelected()
{
    Q_Q(UCListItem);
    bool checked = selectionPanel->property("checked").toBool();
    q->setSelected(checked);
    update();
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
    // rebound to zero
    animator->snap(0);
}

void UCListItemPrivate::_q_updateIndex()
{
    Q_Q(UCListItem);
    q->update();
}

/*!
 * \qmlproperty Component ListItem::style
 * Holds the style of the component defining the components visualizing the leading/
 * trailing actions, selection and dragging mode handlers as well as different
 * animations. The component does not assume any visuals present in the style,
 * and will load its content only when requested, i.e. either of the mentioned
 * components are visualized.
 */
QQmlComponent *UCListItemPrivate::style() const
{
    return styleComponent;
}
void UCListItemPrivate::setStyle(QQmlComponent *delegate)
{
    if (styleComponent == delegate) {
        return;
    }
    Q_Q(UCListItem);
    // make sure we're rebound before we change the panel component
    promptRebound();
    if (styleItem) {
        delete styleItem;
        styleItem = 0;
        Q_EMIT q->__styleInstanceChanged();
    }
    delete styleComponent;
    customStyle = (delegate == 0);
    styleComponent = delegate;
    loadStyle();
    Q_EMIT q->styleChanged();
}

// update themed components
bool UCListItemPrivate::loadStyle()
{
    if (!ready) {
        return false;
    }
    if (!customStyle && !styleComponent) {
        Q_Q(UCListItem);
        if (styleItem) {
            delete styleItem;
            styleItem = 0;
            Q_EMIT q->__styleInstanceChanged();
        }
        delete styleComponent;
        styleComponent = UCTheme::instance().createStyleComponent("ListItemStyle.qml", q);
    }
    return (styleComponent != NULL);
}
// creates the style item
void UCListItemPrivate::initStyleItem()
{
    if (!styleComponent || styleItem) {
        return;
    }
    Q_Q(UCListItem);
    QObject *object = styleComponent->beginCreate(qmlContext(q));
    styleItem = qobject_cast<UCListItemStyle*>(object);
    if (!styleItem) {
        delete object;
    }
    QQml_setParent_noEvent(styleItem, q);
    styleComponent->completeCreate();
}

/*!
 * \qmlproperty Item ListItem::__styleInstance
 * \internal
 */
QQuickItem *UCListItemPrivate::styleInstance() const
{
    return styleItem;
}

// the function performs a prompt rebound on mouse release without any animation
void UCListItemPrivate::promptRebound()
{
    // stop any animation and rebound
    animator->complete();
    setPressed(false);
    setTugged(false);
    animator->snapOut();
}

// set pressed flag and update background
// called when units size changes
void UCListItemPrivate::_q_updateSize()
{
    Q_Q(UCListItem);
    QQuickItem *owner = flickable ? flickable : parentItem;
    q->setImplicitWidth(owner ? owner->width() : UCUnits::instance().gu(40));
    q->setImplicitHeight(UCUnits::instance().gu(7));
    // update overshoot value
    overshoot = UCUnits::instance().gu(2);
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

// returns true if the highlight is possible
bool UCListItemPrivate::canHighlight(QMouseEvent *event)
{
    if (highlight == UCListItem::PermanentHighlight) {
        return true;
    }
    if (highlight == UCListItem::DisabledHighlight) {
        return false;
    }
    // if automatic, the highlight should not happen if we clicked on an active component;
    // localPos is a position relative to ListItem which will give us a child from
    // from the original coordinates; therefore we must map the position to the contentItem
    Q_Q(UCListItem);
    QPointF myPos = q->mapToItem(contentItem, event->localPos());
    QQuickItem *child = contentItem->childAt(myPos.x(), myPos.y());
    bool activeComponent = child && (child->acceptedMouseButtons() & event->button()) && !qobject_cast<QQuickText*>(child);
    // do highlight if not pressed above the active component, and the component has either
    // action, leading or trailing actions list or at least an active child component declared
    QQuickMouseArea *ma = q->findChild<QQuickMouseArea*>();
    bool activeMouseArea = ma && ma->isEnabled();
    return !activeComponent && (action || leadingActions || trailingActions || activeMouseArea);
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

// grabs the panels from the leading/trailing actions and disables all ascending flickables
bool UCListItemPrivate::grabPanel(UCListItemActions *actionsList, bool isTugged)
{
    Q_Q(UCListItem);
    if (isTugged) {
        bool grab = UCListItemActionsPrivate::connectToListItem(actionsList, q, (actionsList == leadingActions));
        if (attachedProperties) {
            attachedProperties->disableInteractive(q, true);
        }
        return grab;
    } else {
        UCListItemActionsPrivate::disconnectFromListItem(actionsList);
        return false;
    }
}


// connects/disconnects from the Flickable anchestor to get notified when to do rebound
void UCListItemPrivate::listenToRebind(bool listen)
{
    if (attachedProperties) {
        Q_Q(UCListItem);
        attachedProperties->listenToRebind(q, listen);
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

// clamps the X value and moves the contentItem to the new X value
void UCListItemPrivate::clampAndMoveX(qreal &x, qreal dx)
{
    UCListItemActionsPrivate *leading = UCListItemActionsPrivate::get(leadingActions);
    UCListItemActionsPrivate *trailing = UCListItemActionsPrivate::get(trailingActions);
    x += dx;
    // min cannot be less than the trailing's panel width
    qreal min = (trailing && trailing->panelItem) ? -trailing->panelItem->width() - overshoot: 0;
    // max cannot be bigger than 0 or the leading's width in case we have leading panel
    qreal max = (leading && leading->panelItem) ? leading->panelItem->width() + overshoot: 0;
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
        promptRebound();
        animator->snap(selectionPanel->width());
        // sync selected flag with the attached selection array
        if (attachedProperties) {
            q->setSelected(UCListItemAttachedPrivate::get(attachedProperties)->isItemSelected(q));
        }
        QObject::connect(selectionPanel, SIGNAL(checkedChanged()), q, SLOT(_q_updateSelected()));
    } else {
        // remove content item dimming and destroy selection panel as well
        animator->snap(0.0);
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
 * The ListItem component was designed to be used in a list view. It does not
 * define any specific layout, but while its contents can be freely chosen by
 * the developer, care must be taken to keep the contents light in order to ensure
 * good performance when used in long list views.
 *
 * The component provides two color properties which configures the item's background
 * when normal or pressed. This can be configured through \l color and \l highlightColor
 * properties.
 *
 * \c contentItem holds all components and resources declared as child to ListItem.
 * Being an Item, all properties can be accessed or altered. However, make sure you
 * never change \c x, \c y, \c width, \c height or \c anchors properties as those are
 * controlled by the ListItem itself when leading or trailing actions are revealed
 * and thus might cause the component to misbehave.
 *
 * Each ListItem has a thin divider shown on the bottom of the component. This
 * divider can be configured through the \c divider grouped property, which can
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
 * is set. The signal is not emitted if the ListItem content is swiped or when used in
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

UCListItemAttached *UCListItem::qmlAttachedProperties(QObject *owner)
{
    return new UCListItemAttached(owner);
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
    if (d->attachedProperties) {
        setSelected(UCListItemAttachedPrivate::get(d->attachedProperties)->isItemSelected(this));
    }
}

void UCListItem::itemChange(ItemChange change, const ItemChangeData &data)
{
    UCStyledItemBase::itemChange(change, data);
    if (change == ItemParentHasChanged) {
        Q_D(UCListItem);
        // make sure we are not connected to any previous Flickable
        d->listenToRebind(false);
        // check if we are in a positioner, and if that positioner is in a Flickable
        QQuickBasePositioner *positioner = qobject_cast<QQuickBasePositioner*>(data.item);
        if (positioner && positioner->parentItem()) {
            // count owner is a positioner
            d->flickable = qobject_cast<QQuickFlickable*>(positioner->parentItem()->parentItem());
        } else if (data.item && data.item->parentItem()){
            // check if we are in a Flickable then
            d->flickable = qobject_cast<QQuickFlickable*>(data.item->parentItem());
        }

        // attach ListItem properties to the flickable or to the parent item
        if (d->flickable) {
            // connect to flickable to get width changes
            d->attachedProperties = static_cast<UCListItemAttached*>(qmlAttachedPropertiesObject<UCListItem>(d->flickable));
        } else if (data.item) {
            d->attachedProperties = static_cast<UCListItemAttached*>(qmlAttachedPropertiesObject<UCListItem>(data.item));
        } else {
            // mar as not ready, so no action should be performed which depends on readyness
            d->ready = false;
            // about to be deleted or reparrented, disable attached
            d->attachedProperties = 0;
        }

        if (data.item) {
            QObject::connect(d->flickable ? d->flickable : data.item, SIGNAL(widthChanged()), this, SLOT(_q_updateSize()));
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
    // do highlight only if at least one of action, leadingActions or trailingAction
    // properties is set
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
    if (d->selectable || (d->attachedProperties && d->attachedProperties->isMoving())) {
        // while moving, we cannot select any items
        return;
    }
    if (d->canHighlight(event) && !d->suppressClick
            && !d->pressed && event->button() == Qt::LeftButton) {
        d->setPressed(true);
        d->lastPos = d->pressedPos = event->localPos();
        // connect the Flickable to know when to rebound
        d->listenToRebind(true);
        // start pressandhold timer
        d->pressAndHoldTimer.start(QGuiApplication::styleHints()->mousePressAndHoldInterval(), this);   
        // if it was moved, grab the panels
        if (d->tugged) {
            d->grabPanel(d->leadingActions, true);
            d->grabPanel(d->trailingActions, true);
        }
        // start pressandhold timer
        d->pressAndHoldTimer.start(QGuiApplication::styleHints()->mousePressAndHoldInterval(), this);
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
        d->listenToRebind(false);
        if (d->attachedProperties) {
            d->attachedProperties->disableInteractive(this, false);
        }

        if (!d->suppressClick) {
            Q_EMIT clicked();
            if (d->action) {
                Q_EMIT d->action->trigger(d->index());
            }
            d->_q_rebound();
        } else {
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
            // tries to connect both panels so we do no longer need to take care which
            // got connected ad which not; this may fail in case of shared ListItemActions,
            // as then the panel is shared between the list items, and the panel might be
            // still in use in other panels. See UCListItemActionsPrivate::connectToListItem
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
            d->setContentMoving(true);
            // clamp X into allowed dragging area
            d->clampAndMoveX(x, dx);
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
            // suppress click only if the highlight policy is not PermanentHighlight
            d->suppressClick = (d->highlight != PermanentHighlight);
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
 * of the ListItem. When swiped left or right to reveal the actions, it is not
 * moved together with the content. \c colorFrom and \c colorTo configure
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
 * \qmlproperty enum ListItem::highlightPolicy
 * The property defines the highlight policy of the ListItem. This can be \c AutomaticHighlight,
 * \c PermanentHighlight or \c DisabledHighlight.
 * When \c AutomaticHighlight is set, the list item will be highlighted when it either has
 * a default action, has a trailing- or leading action list, is selectable and
 * selected or it has an active component declared and the press did not happen
 * above this active component. When \c PermanentHighlight is set, the list item
 * is highlighted whenever it is pressed. The opposite happens when the \c
 * DisabledHighlight is set.
 *
 * The \l clicked and \l pressAndHold signals are also driven by this property.
 * This means that when the policy is set to \c DisabledHighlight, these signals
 * will be suppressed.
 */
UCListItem::HighlightPolicy UCListItemPrivate::highlightPolicy() const
{
    return highlight;
}
void UCListItemPrivate::setHighlightPolicy(UCListItem::HighlightPolicy policy)
{
    if (policy == highlight) {
        return;
    }
    highlight = policy;
    Q_Q(UCListItem);
    Q_EMIT q->highlightPolicyChanged();
}

/*!
 * \qmlproperty bool ListItem::contentMoving
 * \readonly
 * The property describes whether the content is moving or not. The content is
 * moved when swiped or when snapping in or out, and lasts till the snapping
 * animation completes.
 */

/*!
 * \qmlsignal ListItem::contentMovementStarted()
 * The signal is emitted when the content movement has started.
 */

/*!
 * \qmlsignal UCListItemPrivate::contentMovementEnded
 * The signal is emitted when the content movement has ended.
 */
bool UCListItemPrivate::contentMoving() const
{
    return contentMoved;
}
void UCListItemPrivate::setContentMoving(bool moved)
{
    if (contentMoved == moved) {
        return;
    }
    contentMoved = moved;
    Q_Q(UCListItem);
    if (contentMoved) {
        Q_EMIT q->contentMovementStarted();
    } else {
        Q_EMIT q->contentMovementEnded();
    }
    Q_EMIT q->contentMovingChanged();
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
 * Configures the color when highlighted. Defaults to the theme palette's background
 * color.
 *
 * An item is highlighted when selected, or when pressed and it has \l leadingActions
 * or \l trailingActions set, when there is a valid default \l action attached
 * to it or if the ListItem has an active component child, such as a \l Button,
 * a \l Switch, or in general, if an active (enabled and visible) \c MouseArea
 * is added as a child component.
 *
 * \note Note that clicking on an active component will highlight the ListItem
 * only if \l highlightPolicy is set to \c PermanentHighlight.
 *
 * \note Adding an active component does not mean the component will be activated
 * when the ListItem will be tapped/clicked outside of the component area. If
 * such a behavior is needed, this must be done explicitly.
 * \qml
 * import QtQiock 2.3
 * import Ubuntu.Components 1.2
 *
 * ListItem {
 *     Label {
 *         text: "This is a label"
 *     }
 *     Switch {
 *         id: toggle
 *         anchors.right: parent.right
 *     }
 *     Component.onCompleted: clicked.connect(toggle.clicked)
 * }
 * \endqml
 *
 * \sa action, leadingActions, trailingActions, highlightPolicy
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
    d->customColor = true;
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
    if (d->attachedProperties) {
        if (selected) {
            UCListItemAttachedPrivate::get(d->attachedProperties)->addSelectedItem(this);
        } else {
            UCListItemAttachedPrivate::get(d->attachedProperties)->removeSelectedItem(this);
        }
    }
    // update panel as well
    if (d->selectionPanel) {
        d->selectionPanel->setProperty("checked", d->selected);
    }
    Q_EMIT selectedChanged();
}

/*!
 * \qmlproperty Action ListItem::action
 * The property holds the default action attached to the list item which will be
 * triggered when the ListItem is clicked. ListItem will not visualize the action,
 * that is the responsibility of the components placed inside the list item.
 * However, when set, the ListItem will be highlighted on press.
 *
 * If the action set has no value type set, ListItem will set its type to \c
 * Action.Integer and the \l {Action::triggered}{triggered} signal will be getting
 * the ListItem index as \e value parameter.
 *
 * \note Handling pressAndHold will suppress the action triggering as the clicked
 * signal is also suppressed. If the action triggering is still needed, it must be
 * triggered manually on \l pressed changed.
 * \qml
 * import Ubuntu.Components 1.2
 *
 * ListItem {
 *     property bool emitActionTriggered: false
 *     action: Action {
 *         onTriggered: console.log("action triggered", value)
 *     }
 *     onPresseAndHold: {
 *         console.log("suppresses clicked() signal, also action triggered");
 *         emitActionTriggered = true;
 *     }
 *     onPressedChanged: {
 *         if (!pressed && emitActionTriggered) {
 *             emitActionTriggered = false;
 *             action.trigger(index);
 *         }
 *     }
 * }
 * \endqml
 *
 * Defaults no null.
 */
UCAction *UCListItem::action() const
{
    Q_D(const UCListItem);
    return d->action;
}
void UCListItem::setAction(UCAction *action)
{
    Q_D(UCListItem);
    if (d->action == action) {
        return;
    }
    d->action = action;
    if (d->action && (d->action->property("parameterType").toInt() == UCAction::None)) {
        d->action->setProperty("parameterType", UCAction::Integer);
    }
    Q_EMIT actionChanged();
}


/*!
 * \qmlproperty list<Object> ListItem::listItemData
 * \default
 * \internal
 * Overloaded default property containing all the children and resources.
 */
QQmlListProperty<QObject> UCListItemPrivate::data()
{
    return QQuickItemPrivate::get(contentItem)->data();
}

/*!
 * \qmlproperty list<Item> ListItem::listItemChildren
 * \internal
 * Overloaded default property containing all the visible children of the item.
 */
QQmlListProperty<QQuickItem> UCListItemPrivate::children()
{
    return QQuickItemPrivate::get(contentItem)->children();
}

#include "moc_uclistitem.cpp"
