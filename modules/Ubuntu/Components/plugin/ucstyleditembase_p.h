/*
 * Copyright 2015 Canonical Ltd.
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
 *
 * Author: Zsombor Egri <zsombor.egri@canonical.com>
 */

#ifndef UCSTYLEDITEMBASE_P_H
#define UCSTYLEDITEMBASE_P_H

#include <QtQuick/private/qquickitem_p.h>
#include "ucstyleditembase.h"

class QQuickMouseArea;
class UCStyledItemBase;
class UCStyledItemBasePrivate : public QQuickItemPrivate
{
    Q_DECLARE_PUBLIC(UCStyledItemBase)
public:

    static UCStyledItemBasePrivate *get(UCStyledItemBase *item) {
        return item->d_func();
    }

    void _q_ascendantChanged(QQuickItem *ascendant);
    void _q_parentStyleChanged();

    UCStyledItemBasePrivate();
    virtual ~UCStyledItemBasePrivate();
    void init();

    virtual void setFocusable(bool focus);
    bool isParentFocusable();

    UCTheme *getTheme() const;
    void setTheme(UCTheme *styleSet);
    void resetTheme();

public:
    bool activeFocusOnPress:1;
    bool subthemingEnabled:1;
    UCTheme *theme;
    QPointer<UCStyledItemBase> parentStyledItem;

private:
    QStack< QPointer<QQuickItem> > parentStack;

    bool connectParents(QQuickItem *fromItem);
    bool setParentStyled(UCStyledItemBase *styledItem);
    void disconnectTillItem(QQuickItem *item);
};

#endif // UCSTYLEDITEMBASE_P_H
