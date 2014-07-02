/*
 * Copyright 2013 Canonical Ltd.
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
 *          Florian Boucault <florian.boucault@canonical.com>
 */

#include "ucthemesettings.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStandardPaths>

/*
  User theme settings are stored in $XDG_CONFIG_HOME/ubuntu-ui-toolkit/theme.ini file, which contains
  the current global theme name.
*/

const QString SETTINGS_FILE_FORMAT("%1/ubuntu-ui-toolkit/theme.ini");
const QString THEME_KEY("theme");
const QString DEFAULT_THEME("Ubuntu.Components.Themes.Ambiance");

UCThemeSettings::UCThemeSettings(QObject *parent) :
    QObject(parent),
    m_settings(SETTINGS_FILE_FORMAT.arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)), QSettings::IniFormat)
{
    // check if there is a theme settings file, if not, create one
    if (!QFile::exists(m_settings.fileName())) {
        // create the file by writing the default theme
        m_settings.setValue(THEME_KEY, DEFAULT_THEME);
        m_settings.sync();
    }

    QObject::connect(&m_settingsFileWatcher, &QFileSystemWatcher::fileChanged,
                     this, &UCThemeSettings::reloadSettings);
    reloadSettings();
}

void UCThemeSettings::reloadSettings()
{
    m_settings.sync();
    m_settingsFileWatcher.addPath(m_settings.fileName());

    QString themeName = m_settings.value(THEME_KEY).toString();
    if (themeName != m_themeName) {
        m_themeName = themeName;
        Q_EMIT themeNameChanged();
    }
}

QString UCThemeSettings::themeName() const
{
    return m_themeName;
}

void UCThemeSettings::setThemeName(const QString &themeName)
{
    if (themeName != m_themeName) {
        m_themeName = themeName;
        m_settings.setValue(THEME_KEY, themeName);
        Q_EMIT themeNameChanged();
    }
}
