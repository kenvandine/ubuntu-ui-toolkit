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
 * Authors: Zsombor Egri <zsombor.egri@canonical.com>
 */

#ifndef UCDEPRECATEDTHEME_H
#define UCDEPRECATEDTHEME_H

#include <QtCore/QObject>

class QQmlComponent;
class QQmlContext;
class UCTheme;
class UCDeprecatedTheme : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName RESET resetName NOTIFY nameChanged)
    Q_PROPERTY(QObject* palette READ palette NOTIFY paletteChanged)
public:
    static UCDeprecatedTheme& instance() {
        static UCDeprecatedTheme instance;
        return instance;
    }
    explicit UCDeprecatedTheme(QObject *parent = 0);

    // getter/setters
    QString name();
    void setName(const QString& name);
    void resetName();
    QObject* palette();

    Q_INVOKABLE QQmlComponent* createStyleComponent(const QString& styleName, QObject* parent);
    void registerToContext(QQmlContext* context);

Q_SIGNALS:
    void nameChanged();
    void paletteChanged();

private:
    void showDeprecatedNote(const char *note);
};

#endif // UCDEPRECATEDTHEME_H
