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

#ifndef UCVIEWITEM_P_H
#define UCVIEWITEM_P_H

#include "uclistitem.h"
#include "ucstyleditembase_p.h"
#include <QtCore/QPointer>
#include <QtCore/QBasicTimer>
#include <QtQuick/private/qquickrectangle_p.h>

#define MIN(x, y)           ((x < y) ? x : y)
#define MAX(x, y)           ((x > y) ? x : y)
#define CLAMP(v, min, max)  (min <= max) ? MAX(min, MIN(v, max)) : MAX(max, MIN(v, min))

#define IMPLICIT_LISTITEM_WIDTH_GU      40
#define IMPLICIT_LISTITEM_HEIGHT_GU     7
#define DIVIDER_THICKNESS_DP            2
#define DEFAULT_SWIPE_THRESHOLD_GU      1.5
#define DRAG_SCROLL_TIMEOUT             15

class QQuickFlickable;
class QQuickPropertyAnimation;
class UCListItemContent;
class UCListItemDivider;
class UCListItemActions;
class UCListItemSnapAnimator;
class UCListItemStyle;
class UCSelectionHandler;
class UCDragHandler;
class UCActionPanel;
class UCListItemPrivate : public UCStyledItemBasePrivate
{
    Q_DECLARE_PUBLIC(UCListItem)
public:
    UCListItemPrivate();
    virtual ~UCListItemPrivate();
    void init();

    static inline UCListItemPrivate *get(UCListItem *that)
    {
        Q_ASSERT(that);
        return that->d_func();
    }
    inline UCListItem *item()
    {
        return q_func();
    }

    bool isClickedConnected();
    bool isPressAndHoldConnected();
    void _q_updateThemedData();
    void _q_rebound();
    void promptRebound();
    void _q_updateSize();
    void _q_updateIndex();
    int index();
    bool canHighlight(QMouseEvent *event);
    void setHighlighted(bool pressed);
    void setSwiped(bool tugged);
    void listenToRebind(bool listen);
    void lockContentItem(bool lock);
    void adjustContentItemHeight();
    void update();
    void clampAndMoveX(qreal &x, qreal dx);

    bool highlighted:1;
    bool contentMoved:1;
    bool swiped:1;
    bool suppressClick:1;
    bool ready:1;
    bool customColor:1;
    bool customOvershoot:1;
    qreal xAxisMoveThresholdGU;
    qreal overshoot;
    QBasicTimer pressAndHoldTimer;
    QPointF lastPos;
    QPointF pressedPos;
    QColor color;
    QColor highlightColor;
    QPointer<QQuickItem> countOwner;
    QPointer<QQuickFlickable> flickable;
    QPointer<UCViewItemsAttached> parentAttached;
    QQuickItem *contentItem;
    UCListItemDivider *divider;
    UCListItemActions *leadingActions;
    UCListItemActions *trailingActions;
    UCActionPanel *leadingPanel;
    UCActionPanel *trailingPanel;
    UCListItemSnapAnimator *animator;
    UCAction *defaultAction;
    UCSelectionHandler *selectionHandler;
    UCDragHandler *dragHandler;

    // FIXME move these to StyledItemBase togehther with subtheming.
    QQmlComponent *styleComponent;
    QQmlComponent *implicitStyleComponent;
    UCListItemStyle *styleItem;

    // getters/setters
    qreal swipeOvershoot() const;
    void setSwipeOvershoot(qreal overshoot);
    void resetSwipeOvershoot();
    QQmlListProperty<QObject> data();
    QQmlListProperty<QQuickItem> children();
    bool contentMoving() const;
    void setContentMoving(bool moved);
    QQmlComponent *style() const;
    void setStyle(QQmlComponent *delegate);
    void resetStyle();
    void initStyleItem();
    QQuickItem *styleInstance() const;
    bool dragging();
    bool isDraggable();
    void _q_initializeDragHandler();
    bool isSelected();
    void setSelected(bool value);
    bool isSelectable();
    void _q_initializeSelectionHandler();
    UCAction *action() const;
    void setAction(UCAction *action);
};

class UCListItemAttachedPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(UCListItemAttached)
public:
    UCListItemAttachedPrivate() : QObjectPrivate(), panel(0), listItem(0), animate(true) {}

    static UCListItemAttachedPrivate* get(UCListItemAttached *that)
    {
        return that->d_func();
    }
    void setAnimate(bool value);

    UCActionPanel *panel;
    UCListItem *listItem;
    QList<UCAction*> visibleActions;
    bool animate:1;
};

class PropertyChange;
class UCViewItemsAttachedPrivate
{
    Q_DECLARE_PUBLIC(UCViewItemsAttached)
public:
    UCViewItemsAttachedPrivate(UCViewItemsAttached *qq);
    ~UCViewItemsAttachedPrivate();

    static UCViewItemsAttachedPrivate *get(UCViewItemsAttached *item)
    {
        return item ? item->d_func() : 0;
    }

    void clearFlickablesList();
    void buildFlickablesList();
    void clearChangesList();
    void buildChangesList(const QVariant &newValue);
    bool addSelectedItem(UCListItem *item);
    bool removeSelectedItem(UCListItem *item);
    bool isItemSelected(UCListItem *item);

    // dragging mode functions
    void enterDragMode();
    void leaveDragMode();
    bool isDraggingStartedConnected();
    bool isDraggingUpdatedConnected();

    QPointF mapDragAreaPos();
    int indexAt(qreal x, qreal y);
    UCListItem *itemAt(qreal x, qreal y);
    void createDraggedItem(UCListItem *dragItem);
    void updateDraggedItem();
    void updateSelectedIndexes(int fromIndex, int toIndex);

    UCViewItemsAttached *q_ptr;
    QQuickFlickable *listView;
    bool ready:1;
    bool globalDisabled:1;
    bool selectable:1;
    bool draggable:1;
    QSet<int> selectedList;
    QList< QPointer<QQuickFlickable> > flickables;
    QList< PropertyChange* > changes;
    QPointer<UCListItem> boundItem;
    QPointer<UCListItem> disablerItem;

    // drag handling
    QBasicTimer dragScrollTimer;
    QQuickMouseArea *dragHandlerArea;
    QPointer<UCListItem> dragTempItem;
    qreal dragLastY;
    UCDragEvent::Direction dragDirection;
    int dragFromIndex;
    int dragToIndex;
    int dragMinimum;
    int dragMaximum;
};

class UCActionPanel : public QObject
{
    Q_OBJECT
public:
    ~UCActionPanel();
    static bool grabPanel(UCActionPanel **panel, UCListItem *item, bool leading);
    static void ungrabPanel(UCActionPanel *panel);
    static bool isConnected(UCActionPanel *panel);

    UCListItemActions *actions();
    QQuickItem *panel() const;
    UCListItem::PanelStatus panelStatus()
    {
        return status;
    }
    bool isLeading() const
    {
        return leading;
    }

Q_SIGNALS:
    void statusChanged();

private:
    UCActionPanel(UCListItem *item, bool leading);
    bool createPanel(QQmlComponent *panelDelegate);
    UCListItemAttached *attachedObject();

    UCListItem *listItem;
    QQuickItem *panelItem;
    UCListItem::PanelStatus status;
    bool leading:1;
    bool connected:1;
};

class UCListItemDivider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool visible MEMBER m_visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(qreal leftMargin MEMBER m_leftMargin WRITE setLeftMargin NOTIFY leftMarginChanged)
    Q_PROPERTY(qreal rightMargin MEMBER m_rightMargin WRITE setRightMargin NOTIFY rightMarginChanged)
    Q_PROPERTY(QColor colorFrom MEMBER m_colorFrom WRITE setColorFrom NOTIFY colorFromChanged)
    Q_PROPERTY(QColor colorTo MEMBER m_colorTo WRITE setColorTo NOTIFY colorToChanged)
public:
    explicit UCListItemDivider(QObject *parent = 0);
    ~UCListItemDivider();
    void init(UCListItem *listItem);

Q_SIGNALS:
    void visibleChanged();
    void leftMarginChanged();
    void rightMarginChanged();
    void colorFromChanged();
    void colorToChanged();

protected:
    QSGNode *paint(QSGNode *node, const QRectF &rect);

private Q_SLOTS:
    void unitsChanged();
    void paletteChanged();

private:
    void updateGradient();
    void setVisible(bool visible);
    void setLeftMargin(qreal leftMargin);
    void setRightMargin(qreal rightMargin);
    void setColorFrom(const QColor &color);
    void setColorTo(const QColor &color);

    bool m_visible:1;
    bool m_colorFromChanged:1;
    bool m_colorToChanged:1;
    qreal m_thickness;
    qreal m_leftMargin;
    qreal m_rightMargin;
    QColor m_colorFrom;
    QColor m_colorTo;
    QGradientStops m_gradient;
    UCListItemPrivate *m_listItem;
    friend class UCListItem;
    friend class UCListItemPrivate;
};

QColor getPaletteColor(const char *profile, const char *color);

QML_DECLARE_TYPE(UCListItemDivider)

class QQuickPropertyAnimation;
class UCListItemSnapAnimator : public QObject
{
    Q_OBJECT
public:
    UCListItemSnapAnimator(UCListItem *item);
    ~UCListItemSnapAnimator();

    bool snap(qreal to);
    void stop();
    void complete();

public Q_SLOTS:
    void snapOut();
    void snapIn();

    QQuickPropertyAnimation *getDefaultAnimation();

private:
    bool active;
    UCListItem *item;
};

class UCHandlerBase : public QObject
{
    Q_OBJECT
public:

    explicit UCHandlerBase(UCListItem *owner = 0);
    virtual void initialize(bool animated) = 0;

protected:
    UCListItem *listItem;
    QQuickItem *panel;

    void setupPanel(QQmlComponent *component, bool animate);
};

class UCSelectionHandler : public UCHandlerBase
{
    Q_OBJECT
public:
    explicit UCSelectionHandler(UCListItem *owner = 0);

    void initialize(bool animated);

public Q_SLOTS:
    void setupSelection(bool animated = true);
};

class UCDragHandler : public UCHandlerBase
{
    Q_OBJECT
public:
    explicit UCDragHandler(UCListItem *listItem);
    ~UCDragHandler();

    void initialize(bool animated);
    bool isDragging()
    {
        return dragging;
    }
    void setDragging(bool value);

    bool isPanel(QQuickItem *item)
    {
        return item == panel;
    }

    void startDragging(UCListItem *item);
    void drop();
    void update(UCListItem *newItem);

Q_SIGNALS:
    void draggingChanged();

public Q_SLOTS:
    void setupDragMode(bool animated = true);
    void repositionDraggedItem();

protected:
    bool dragging:1;
    bool isDraggedItem:1;
    QPointer<UCListItem> originalItem;
    PropertyChange *visibleProperty;
    QQuickPropertyAnimation *repositionAnimation;
};

#endif // UCVIEWITEM_P_H
