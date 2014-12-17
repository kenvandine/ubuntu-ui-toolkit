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

#define IMPLICIT_LISTITEM_WIDTH_GU      40
#define IMPLICIT_LISTITEM_HEIGHT_GU     7
#define DIVIDER_LEFT_MARGIN_DP          2
#define DIVIDER_RIGHT_MARGIN_DP         2
#define DIVIDER_THICKNESS_DP            2
#define DEFAULT_SWIPE_THRESHOLD_GU      1.5

class QQuickFlickable;
class QQuickPropertyAnimation;
class UCListItemContent;
class UCListItemDivider;
class UCListItemActions;
class UCListItemSnapAnimator;
class UCListItemStyle;
class UCSelectionHandler;
class UCDragHandler;
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
    void setPressed(bool pressed);
    void setSwiped(bool tugged);
    bool grabPanel(UCListItemActions *optionList, bool isTugged);
    void listenToRebind(bool listen);
    void resize();
    void update();
    void clampAndMoveX(qreal &x, qreal dx);

    bool pressed:1;
    bool contentMoved:1;
    bool highlightColorChanged:1;
    bool swiped:1;
    bool suppressClick:1;
    bool ready:1;
    bool customStyle:1;
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
    QPointer<UCListItemAttached> attachedProperties;
    QQuickItem *contentItem;
    UCListItemDivider *divider;
    UCListItemActions *leadingActions;
    UCListItemActions *trailingActions;
    UCListItemSnapAnimator *animator;
    UCAction *defaultAction;
    UCSelectionHandler *selectionHandler;
    UCDragHandler *dragHandler;

    // FIXME move these to StyledItemBase togehther with subtheming.
    QQmlComponent *styleComponent;
    UCListItemStyle *styleItem;

    // getters/setters
    qreal swipeOvershoot() const;
    void setSwipeOvershoot(qreal overshoot);
    QQmlListProperty<QObject> data();
    QQmlListProperty<QQuickItem> children();
    bool contentMoving() const;
    void setContentMoving(bool moved);
    QQmlComponent *style() const;
    void setStyle(QQmlComponent *delegate);
    bool loadStyle(bool reload);
    void initStyleItem();
    QQuickItem *styleInstance() const;
    bool dragging();
    bool isDraggable();
    bool isSelected() const;
    void setSelected(bool value);
    bool isSelectable();
    UCAction *action() const;
    void setAction(UCAction *action);
};

class PropertyChange;
class UCListItemAttachedPrivate
{
    Q_DECLARE_PUBLIC(UCListItemAttached)
public:
    UCListItemAttachedPrivate(UCListItemAttached *qq);
    ~UCListItemAttachedPrivate();

    static UCListItemAttachedPrivate *get(UCListItemAttached *item)
    {
        return item ? item->d_func() : 0;
    }

    void clearFlickablesList();
    void buildFlickablesList();
    void clearChangesList();
    void buildChangesList(const QVariant &newValue);
    void addSelectedItem(UCListItem *item);
    void removeSelectedItem(UCListItem *item);
    bool isItemSelected(UCListItem *item);

    // dragging mode functions
    void enterDragMode();
    void leaveDragMode();
    bool isDraggingStartedConnected();
    bool isDraggingUpdatedConnected();

    qreal dragY();
    int indexAt(qreal x, qreal y);
    UCListItem *itemAt(qreal x, qreal y);
    void createDraggedItem();

    UCListItemAttached *q_ptr;
    QQuickFlickable *listView;
    QQuickMouseArea *dragHandlerArea;
    bool globalDisabled:1;
    bool selectable:1;
    bool draggable:1;
    bool dragSuspended:1;
    QList<int> selectedList;
    QList< QPointer<QQuickFlickable> > flickables;
    QList< PropertyChange* > changes;
    QPointer<UCListItem> boundItem;
    QPointer<UCListItem> disablerItem;

    // drag handling
    PropertyChange *dragVisible;
    UCListItem *dragTempItem;
    UCListItem *dragItem;
    qreal dragLastY;
    int dragIndex;
    int dragNewIndex;
    int dragCurrentId;
    int dragMinimum;
    int dragMaximum;

    // getter/setter
    bool selectMode() const;
    void setSelectMode(bool value);
    QList<int> selectedIndexes() const;
    void setSelectedIndexes(const QList<int> &list);
    bool dragMode() const;
    void setDragMode(bool value);
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
    bool m_leftMarginChanged:1;
    bool m_rightMarginChanged:1;
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
    virtual void initialize() = 0;

protected:
    UCListItemPrivate *listItem;
    QQuickItem *panel;

    void setupPanel(QQmlComponent *component, bool animate);

};

class UCSelectionHandler : public UCHandlerBase
{
    Q_OBJECT
public:
    explicit UCSelectionHandler(UCListItem *owner = 0);

    void initialize();
    bool isSelected()
    {
        return selected;
    }
    void setSelected(bool value);

public Q_SLOTS:
    void setupSelection();

protected:
    bool selected:1;
};

class UCDragHandler : public UCHandlerBase
{
    Q_OBJECT
public:
    explicit UCDragHandler(UCListItem *listItem);

    void initialize();
    bool isDragging()
    {
        return dragging;
    }
    void setDragging(bool value)
    {
        if (dragging == value) {
            return;
        }
        dragging = value;
        Q_EMIT listItem->item()->draggingChanged();
    }

    bool isPanel(QQuickItem *item)
    {
        return item == panel;
    }

Q_SIGNALS:
    void draggingChanged();

public Q_SLOTS:
    void setupDragMode();

protected:
    QPointF lastPos;
    bool dragging:1;

    QPointF panelCenterToListView();
    QPointF mapMousePosToListView(QEvent *event);
    QPointF deltaPos(const QPointF &pos);
};

#endif // UCVIEWITEM_P_H
