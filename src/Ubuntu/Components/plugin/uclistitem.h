/*
 * Copyright 2014-2015 Canonical Ltd.
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

#ifndef UCLISTITEM_H
#define UCLISTITEM_H

#include <QtQuick/QQuickItem>
#include "ucstyleditembase.h"

class UCListItemContent;
class UCListItemDivider;
class UCListItemActions;
class UCAction;
class UCListItemPrivate;
class UCListItem : public UCStyledItemBase
{
    Q_OBJECT
    Q_PROPERTY(QQuickItem *contentItem READ contentItem CONSTANT)
    Q_PROPERTY(UCListItemDivider *divider READ divider CONSTANT)
    Q_PROPERTY(UCListItemActions *leadingActions READ leadingActions WRITE setLeadingActions NOTIFY leadingActionsChanged DESIGNABLE false)
    Q_PROPERTY(UCListItemActions *trailingActions READ trailingActions WRITE setTrailingActions NOTIFY trailingActionsChanged DESIGNABLE false)
    Q_PROPERTY(bool highlighted READ highlighted NOTIFY highlightedChanged)
    Q_PRIVATE_PROPERTY(UCListItem::d_func(), bool contentMoving READ contentMoving NOTIFY contentMovingChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QColor highlightColor READ highlightColor WRITE setHighlightColor RESET resetHighlightColor NOTIFY highlightColorChanged)
    Q_PRIVATE_PROPERTY(UCListItem::d_func(), bool dragging READ dragging NOTIFY draggingChanged)
    Q_PRIVATE_PROPERTY(UCListItem::d_func(), bool dragMode READ dragMode WRITE setDragMode NOTIFY dragModeChanged)
    Q_PRIVATE_PROPERTY(UCListItem::d_func(), bool selected READ isSelected WRITE setSelected NOTIFY selectedChanged)
    Q_PRIVATE_PROPERTY(UCListItem::d_func(), bool selectMode READ selectMode WRITE setSelectMode NOTIFY selectModeChanged)
    Q_PRIVATE_PROPERTY(UCListItem::d_func(), UCAction *action READ action WRITE setAction NOTIFY actionChanged DESIGNABLE false)
    Q_PRIVATE_PROPERTY(UCListItem::d_func(), QQmlListProperty<QObject> listItemData READ data DESIGNABLE false)
    Q_PRIVATE_PROPERTY(UCListItem::d_func(), QQmlListProperty<QQuickItem> listItemChildren READ children NOTIFY listItemChildrenChanged DESIGNABLE false)
    Q_CLASSINFO("DefaultProperty", "listItemData")
public:
    explicit UCListItem(QQuickItem *parent = 0);
    ~UCListItem();

    QQuickItem *contentItem() const;
    UCListItemDivider *divider() const;
    UCListItemActions *leadingActions() const;
    void setLeadingActions(UCListItemActions *options);
    UCListItemActions *trailingActions() const;
    void setTrailingActions(UCListItemActions *options);
    bool highlighted() const;
    QColor color() const;
    void setColor(const QColor &color);
    QColor highlightColor() const;
    void setHighlightColor(const QColor &color);
    void resetHighlightColor();

protected:
    virtual QObject *attachedViewItems(QObject *object, bool create);
    void classBegin();
    void componentComplete();
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data);
    void itemChange(ItemChange change, const ItemChangeData &data);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    bool childMouseEventFilter(QQuickItem *child, QEvent *event);
    bool eventFilter(QObject *, QEvent *);
    void timerEvent(QTimerEvent *event);

Q_SIGNALS:
    void leadingActionsChanged();
    void trailingActionsChanged();
    void highlightedChanged();
    void contentMovingChanged();
    void colorChanged();
    void highlightColorChanged();
    void draggingChanged();
    void dragModeChanged();
    void selectedChanged();
    void selectModeChanged();
    void actionChanged();
    void listItemChildrenChanged();

    void clicked();
    void pressAndHold();

    void contentMovementStarted();
    void contentMovementEnded();

public Q_SLOTS:

protected:
    Q_DECLARE_PRIVATE(UCListItem)

private:
    Q_PRIVATE_SLOT(d_func(), void _q_themeChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_relayout())
    Q_PRIVATE_SLOT(d_func(), void _q_updateSwiping())
    Q_PRIVATE_SLOT(d_func(), void _q_updateSize())
    Q_PRIVATE_SLOT(d_func(), void _q_updateIndex())
    Q_PRIVATE_SLOT(d_func(), void _q_contentMoving())
    Q_PRIVATE_SLOT(d_func(), void _q_syncSelectMode())
    Q_PRIVATE_SLOT(d_func(), void _q_syncDragMode())
};

class UCListItemExpansion;
class UCListItem13 : public UCListItem
{
    Q_OBJECT
    Q_PROPERTY(UCListItemExpansion* expansion READ expansion CONSTANT)
protected:
    virtual QObject *attachedViewItems(QObject *object, bool create);
    void itemChange(ItemChange change, const ItemChangeData &data);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
private:
    Q_SLOT void _q_updateExpansion(const QList<int> &indices);
    bool shouldShowContextMenu(QMouseEvent *event);
    void popoverClosed();
public:
    explicit UCListItem13(QQuickItem *parent = 0);

    UCListItemExpansion *expansion();
};

class UCListItemDividerPrivate;
class UCListItemDivider : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QColor colorFrom READ colorFrom WRITE setColorFrom NOTIFY colorFromChanged)
    Q_PROPERTY(QColor colorTo READ colorTo WRITE setColorTo NOTIFY colorToChanged)
public:
    explicit UCListItemDivider(UCListItem *parent = 0);
    ~UCListItemDivider();
    void init(UCListItem *listItem);
    void paletteChanged();

Q_SIGNALS:
    void colorFromChanged();
    void colorToChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data);

private:
    void updateGradient();
    QColor colorFrom() const;
    void setColorFrom(const QColor &color);
    QColor colorTo() const;
    void setColorTo(const QColor &color);
    Q_DECLARE_PRIVATE(UCListItemDivider)
};

class UCDragEvent;
class UCViewItemsAttachedPrivate;
class UCViewItemsAttached : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool selectMode READ selectMode WRITE setSelectMode NOTIFY selectModeChanged)
    Q_PROPERTY(QList<int> selectedIndices READ selectedIndices WRITE setSelectedIndices NOTIFY selectedIndicesChanged)
    Q_PROPERTY(bool dragMode READ dragMode WRITE setDragMode NOTIFY dragModeChanged)
    Q_ENUMS(ExpansionFlag)
public:
    enum ExpansionFlag {
        Exclusive = 0x01,
        LockExpanded = 0x02,
        CollapseOnOutsidePress = Exclusive | 0x04
    };
    Q_DECLARE_FLAGS(ExpansionFlags, ExpansionFlag)
    explicit UCViewItemsAttached(QObject *owner = 0);
    ~UCViewItemsAttached();

    static UCViewItemsAttached *qmlAttachedProperties(QObject *owner);

    bool listenToRebind(UCListItem *item, bool listen);
    void disableInteractive(UCListItem *item, bool disable);
    bool isMoving();
    bool isBoundTo(UCListItem *item);

    // getter/setter
    bool selectMode() const;
    void setSelectMode(bool value);
    QList<int> selectedIndices() const;
    void setSelectedIndices(const QList<int> &list);
    bool dragMode() const;
    void setDragMode(bool value);

private Q_SLOTS:
    void unbindItem();
    void completed();

Q_SIGNALS:
    void selectModeChanged();
    void selectedIndicesChanged();
    void dragModeChanged();

    void dragUpdated(UCDragEvent *event);

private:
    Q_DECLARE_PRIVATE(UCViewItemsAttached)
};
Q_DECLARE_OPERATORS_FOR_FLAGS(UCViewItemsAttached::ExpansionFlags)
QML_DECLARE_TYPEINFO(UCViewItemsAttached, QML_HAS_ATTACHED_PROPERTIES)

// FIXME keep the 1.3 properties in a separate class, workaround for bug
// https://bugs.launchpad.net/ubuntu/+source/qtdeclarative-opensource-src/+bug/1389721
// enums and flag are added to UCViewItemsAttached like normal
class UCViewItemsAttached13 : public UCViewItemsAttached
{
    Q_OBJECT
    Q_PROPERTY(QList<int> expandedIndices READ expandedIndices WRITE setExpandedIndices NOTIFY expandedIndicesChanged)
    Q_PROPERTY(int expansionFlags READ expansionFlags WRITE setExpansionFlags NOTIFY expansionFlagsChanged)
public:
    explicit UCViewItemsAttached13(QObject *owner = 0);
    static UCViewItemsAttached13 *qmlAttachedProperties(QObject *owner);

    QList<int> expandedIndices() const;
    void setExpandedIndices(QList<int> indices);
    int expansionFlags() const;
    void setExpansionFlags(int flags);

Q_SIGNALS:
    void expandedIndicesChanged(const QList<int> &indices);
    void expansionFlagsChanged();

private:
    UCViewItemsAttachedPrivate *d_ptr;
    Q_DECLARE_PRIVATE_D(d_ptr, UCViewItemsAttached)
};
QML_DECLARE_TYPEINFO(UCViewItemsAttached13, QML_HAS_ATTACHED_PROPERTIES)

class UCListItemExpansion : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool expanded READ expanded WRITE setExpanded NOTIFY expandedChanged)
    Q_PROPERTY(qreal height MEMBER m_height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(QQmlComponent *content MEMBER m_content WRITE setContent NOTIFY contentChanged)
    Q_PROPERTY(QQuickItem *contentItem READ contentItem NOTIFY contentItemChanged)
    Q_PROPERTY(bool overlapListItem MEMBER m_overlapListItem WRITE setOverlapListItem NOTIFY overlapListItemChanged)
public:
    explicit UCListItemExpansion(QObject *parent = 0);

    bool expandedWithFlag(UCViewItemsAttached::ExpansionFlag flag);
    void enableClickFiltering(bool enable);

    bool expanded();
    void setExpanded(bool expanded);
    void setHeight(qreal height);
    void setContent(QQmlComponent *component);
    QQuickItem *contentItem();
    void setOverlapListItem(bool overlap);

Q_SIGNALS:
    void expandedChanged();
    void heightChanged();
    void contentChanged();
    void contentItemChanged();
    void overlapListItemChanged();

protected:
    bool eventFilter(QObject *, QEvent *);
    void createOrUpdateContentItem();

private:
    UCListItem13 *m_listItem;
    QQmlComponent *m_content;
    QQuickItem *m_contentItem;
    qreal m_height;
    bool m_overlapListItem:1;
    bool m_filtering:1;

    friend class UCListItem;
    friend class UCListItem13;
    friend class UCListItemPrivate;
};

class UCDragEvent : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Status status READ status)
    Q_PROPERTY(int from READ from)
    Q_PROPERTY(int to READ to)
    Q_PROPERTY(int minimumIndex MEMBER m_minimum)
    Q_PROPERTY(int maximumIndex MEMBER m_maximum)
    Q_PROPERTY(bool accept MEMBER m_accept)
    Q_ENUMS(Status)
public:
    enum Status {
        Started,
        Moving,
        Dropped
    };

    explicit UCDragEvent(Status status, int from, int to, int min, int max)
        : QObject(0), m_status(status), m_from(from), m_to(to), m_minimum(min), m_maximum(max), m_accept(true)
    {}
    int from() const
    {
        return m_from;
    }
    int to() const
    {
        return m_to;
    }
    Status status() const
    {
        return m_status;
    }

private:
    Status m_status;
    int m_from;
    int m_to;
    int m_minimum;
    int m_maximum;
    bool m_accept;

    friend class ListItemDragArea;
};

#endif // UCLISTITEM_H
