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

#ifndef UCLISTITEMACTIONS_H
#define UCLISTITEMACTIONS_H

#include <QtCore/QObject>
#include "uclistitem_p.h"

class UCListItemActionsAttached;
class QQmlComponent;
class UCAction;
class UCListItemActionsAttached;
class UCListItemActionsPrivate;
class UCListItemActions : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlComponent *delegate READ delegate WRITE setDelegate NOTIFY delegateChanged)
    Q_PROPERTY(QQmlComponent *customPanel READ customPanel WRITE setCustomPanel NOTIFY customPanelChanged)
    Q_PROPERTY(QQmlListProperty<UCAction> actions READ actions CONSTANT)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor NOTIFY foregroundColorChanged)
    Q_PROPERTY(QQmlListProperty<QObject> data READ data)
    Q_CLASSINFO("DefaultProperty", "data")
    Q_ENUMS(Status)
public:
    enum Status {
        Disconnected,
        Leading,
        Trailing
    };

    explicit UCListItemActions(QObject *parent = 0);
    ~UCListItemActions();

    static UCListItemActionsAttached *qmlAttachedProperties(QObject *owner);

    QQmlComponent *delegate() const;
    void setDelegate(QQmlComponent *delegate);
    QQmlComponent *customPanel() const;
    void setCustomPanel(QQmlComponent *panel);
    QQmlListProperty<UCAction> actions();
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);
    QColor foregroundColor() const;
    void setForegroundColor(const QColor &color);
    QQmlListProperty<QObject> data();

Q_SIGNALS:
    void delegateChanged();
    void customPanelChanged();
    void backgroundColorChanged();
    void foregroundColorChanged();

    // keep it as private signal
    void __statusChanged();
    void __draggingChanged();

private:
    Q_DECLARE_PRIVATE(UCListItemActions)
    Q_PRIVATE_SLOT(d_func(), void _q_handlePanelDrag())
    Q_PRIVATE_SLOT(d_func(), void _q_handlePanelWidth())
};

class UCListItemActionsAttached : public QObject
{
    Q_OBJECT
    Q_PROPERTY(UCListItemActions *container READ container NOTIFY containerChanged)
    Q_PROPERTY(UCListItem *listItem READ listItem NOTIFY listItemChanged)
    Q_PROPERTY(int listItemIndex READ listItemIndex NOTIFY listItemIndexChanged)
    Q_PROPERTY(qreal offset READ offset NOTIFY offsetChanged)
    Q_PROPERTY(UCListItemActions::Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool dragging READ dragging NOTIFY draggingChanged)
public:
    UCListItemActionsAttached(QObject *parent = 0);
    ~UCListItemActionsAttached();
    void setList(UCListItemActions *list);

    UCListItem *listItem();
    int listItemIndex();
    bool dragging();
    qreal offset();
    UCListItemActions::Status status();

    UCListItemActions *container() const
    {
        return m_container.data();
    }

public Q_SLOTS:
    void snapToPosition(qreal position);

Q_SIGNALS:
    void containerChanged();
    void listItemChanged();
    void listItemIndexChanged();
    void offsetChanged();
    void statusChanged();
    void draggingChanged();

private:
    QPointer<UCListItemActions> m_container;
    friend class UCListItemAction;
};

QML_DECLARE_TYPEINFO(UCListItemActions, QML_HAS_ATTACHED_PROPERTIES)

#endif // UCLISTITEMACTIONS_H
