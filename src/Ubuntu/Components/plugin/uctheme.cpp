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
 *          Florian Boucault <florian.boucault@canonical.com>
 */

#include "ucnamespace.h"
#include "uctheme.h"
#include "listener.h"
#include "quickutils.h"
#include "i18n.h"
#include "ucfontutils.h"
#include "ucstyleditembase_p.h"

#include <QtQml/qqml.h>
#include <QtQml/qqmlinfo.h>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QLibraryInfo>
#include <QtCore/QStandardPaths>
#include <QtGui/QGuiApplication>
#include <QtGui/QFont>

#include <QtQml/private/qqmlproperty_p.h>
#include <QtQml/private/qqmlabstractbinding_p.h>
#define foreach Q_FOREACH
#include <QtQml/private/qqmlbinding_p.h>
#undef foreach

/*!
 * \qmltype ThemeSettings
 * \instantiates UCTheme
 * \inqmlmodule Ubuntu.Components 1.3
 * \since Ubuntu.Components 1.3
 * \ingroup theming
 * \brief The ThemeSettings class provides facilities to define the theme of a
 * StyledItem.
 *
 * A global instance is exposed as the \b theme context property.
 *
 * The theme defines the visual aspect of the Ubuntu components. An application
 * can use one or more theme the same time. The ThemeSettings component provides
 * abilities to change the theme used by the component and all its child components.
 *
 * Changing the theme of the entire application can be achieved by changing
 * the name of the root StyledItem's, i.e. MainView's current theme.
 *
 * \qml
 * import QtQuick 2.4
 * import Ubuntu.Components 1.3
 *
 * MainView {
 *     width: units.gu(40)
 *     height: units.gu(71)
 *
 *     theme.name: "Ubuntu.Components.Themes.Ambiance"
 * }
 * \endqml
 * By default, styled items inherit the theme they use from their closest styled
 * item ancestor. In case the application uses MainView, all components will inherit
 * the theme from the MainView.
 * \qml
 * import QtQuick 2.4
 * import Ubuntu.Components 1.3
 *
 * MainView {
 *     width: units.gu(40)
 *     height: units.gu(71)
 *
 *     Page {
 *         title: "Style test"
 *         Button {
 *             text: theme.name == "Ubuntu.Components.Themes.Ambiance" ?
 *                      "SuruDark" : "Ambiance"
 *             onClicked: theme.name = (text == "Ambiance" ?
 *                      "Ubuntu.Components.Themes.SuruDark" :
 *                      "Ubuntu.Components.Themes.Ambiance")
 *         }
 *     }
 * }
 * \endqml
 * \note In the example above the Button inherits the theme from Page, which
 * inherits it from MainView. Therefore changing the theme name in this way will
 * result in a change of the inherited theme. In case a different theme is desired,
 * a new instance of the ThemeSettings must be created on the styled item desired.
 * \qml
 * import QtQuick 2.4
 * import Ubuntu.Components 1.3
 *
 * MainView {
 *     width: units.gu(40)
 *     height: units.gu(71)
 *
 *     Page {
 *         title: "Style test"
 *         theme: ThemeSettings{}
 *         Button {
 *             text: theme.name == "Ubuntu.Components.Themes.Ambiance" ?
 *                      "SuruDark" : "Ambiance"
 *             onClicked: theme.name = (text == "Ambiance" ?
 *                      "Ubuntu.Components.Themes.SuruDark" :
 *                      "Ubuntu.Components.Themes.Ambiance")
 *         }
 *     }
 * }
 * \endqml
 *
 * The \l createStyleComponent function can be used to create the style for a
 * component. The following example will create the style with the inherited
 * theme.
 * \qml
 * import QtQuick 2.4
 * import Ubuntu.Components 1.3
 * StyledItem {
 *     id: myItem
 *     style: theme.createStyleComponent("MyItemStyle.qml", myItem)
 * }
 * \endqml
 * All styled toolkit components such as \l Button, \l CheckBox, \l Switch, etc.
 * create their style in this way. Note that the style component must be part
 * of the theme, otherwise the style creation will fail.
 *
 * \sa {StyledItem}
 */

const QString THEME_FOLDER_FORMAT("%1/%2/");
const QString PARENT_THEME_FILE("parent_theme");

QStringList themeSearchPath()
{
    QString envPath = QLatin1String(getenv("UBUNTU_UI_TOOLKIT_THEMES_PATH"));
    QStringList pathList = envPath.split(':', QString::SkipEmptyParts);
    if (pathList.isEmpty()) {
        // get the default path list from generic data location, which contains
        // XDG_DATA_DIRS
        QString xdgDirs = QLatin1String(getenv("XDG_DATA_DIRS"));
        if (!xdgDirs.isEmpty()) {
            pathList << xdgDirs.split(':', QString::SkipEmptyParts);
        }
        // ~/.local/share
        pathList << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    }

    // append QML import path(s); we must explicitly support env override here
    QString qml2ImportPath(getenv("QML2_IMPORT_PATH"));
    if (!qml2ImportPath.isEmpty()) {
        pathList << qml2ImportPath.split(':', QString::SkipEmptyParts);
    }
    pathList << QLibraryInfo::location(QLibraryInfo::Qml2ImportsPath).split(':', QString::SkipEmptyParts);
    // fix folders
    QStringList result;
    Q_FOREACH(const QString &path, pathList) {
        if (QDir(path).exists()) {
            result << path + '/';
        }
    }
    // prepend current folder
    result.prepend(QDir::currentPath());
    return result;
}

UCTheme::ThemeRecord pathFromThemeName(QString themeName)
{
    // the first entry from pathList is the app's current folder
    UCTheme::ThemeRecord record(themeName, QUrl(), false, false);
    themeName.replace('.', '/');
    QStringList pathList = themeSearchPath();
    Q_FOREACH(const QString &path, pathList) {
        QString themeFolder = THEME_FOLDER_FORMAT.arg(path, themeName);
        // QUrl needs a trailing slash to understand it's a directory
        QString absoluteThemeFolder = QDir(themeFolder).absolutePath().append('/');
        if (QDir(absoluteThemeFolder).exists()) {
            record.deprecated = QFile::exists(absoluteThemeFolder + "deprecated");
            record.shared = QFile::exists(absoluteThemeFolder + "qmldir");
            record.path = QUrl::fromLocalFile(absoluteThemeFolder);
            break;
        }
    }
    return record;
}

QString parentThemeName(const UCTheme::ThemeRecord& themePath)
{
    QString parentTheme;
    if (!themePath.isValid()) {
        qWarning() << qPrintable(UbuntuI18n::instance().tr("Theme not found: \"%1\"").arg(themePath.name));
    } else {
        QFile file(themePath.path.resolved(PARENT_THEME_FILE).toLocalFile());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            parentTheme = in.readLine();
        }
    }
    return parentTheme;
}

/******************************************************************************
 * Theme::PaletteConfig
 */

// builds configuration list and applies the configuration on the palette
void UCTheme::PaletteConfig::configurePalette(QObject *themePalette)
{
    if (!palette || !themePalette || configured) {
        return;
    }
    if (configList.isEmpty()) {
        // need to build config list first
        buildConfig();
    }
    if (!configList.isEmpty()) {
        apply(themePalette);
    }
}

void UCTheme::PaletteConfig::restorePalette()
{
    if (!palette || configList.isEmpty() || !configured) {
        return;
    }

    for (int i = 0; i < configList.count(); i++) {
        Data &config = configList[i];
        if (!config.paletteProperty.isValid()) {
            continue;
        }

        // restore the config binding to the config target
        if (config.configBinding && config.configBinding->bindingType() == QQmlAbstractBinding::Binding) {
            QQmlBinding *qmlBinding = static_cast<QQmlBinding*>(config.configBinding);
            qmlBinding->removeFromObject();
            qmlBinding->setTarget(config.configProperty);
        }

        if (config.paletteBinding) {
            // restore the binding to the palette
            QQmlAbstractBinding *prev = QQmlPropertyPrivate::setBinding(config.paletteProperty, config.paletteBinding);
            if (prev && prev != config.paletteBinding && prev != config.configBinding) {
                prev->destroy();
            }
            config.paletteBinding->update();
        } else {
            config.paletteProperty.write(config.paletteValue);
        }

        config.paletteProperty = QQmlProperty();
        config.paletteBinding = NULL;
        config.paletteValue.clear();
    }

    configured = false;
}

// build palette configuration list
void UCTheme::PaletteConfig::buildConfig()
{
    if (!palette) {
        return;
    }
    const char *valueSetList[10] = {"normal", "selected"};
    QQmlContext *configContext = qmlContext(palette);

    for (int i = 0; i < 2; i++) {
        const char *valueSet = valueSetList[i];
        QObject *configObject = palette->property(valueSet).value<QObject*>();
        const QMetaObject *mo = configObject->metaObject();

        for (int ii = mo->propertyOffset(); ii < mo->propertyCount(); ii++) {
            const QMetaProperty prop = mo->property(ii);
            QString propertyName = QString("%1.%2").arg(valueSet).arg(prop.name());
            QQmlProperty configProperty(palette, propertyName, configContext);

            // first we need to check whether the property has a binding or not
            QQmlAbstractBinding *binding = QQmlPropertyPrivate::binding(configProperty);
            if (binding) {
                configList << Data(propertyName, configProperty, binding);
            } else {
                QVariant value = configProperty.read();
                QColor color = value.value<QColor>();
                if (color.isValid()) {
                    configList << Data(propertyName, configProperty);
                }
            }
        }
    }
}

// apply configuration on the palette
void UCTheme::PaletteConfig::apply(QObject *themePalette)
{
    QQmlContext *context = qmlContext(themePalette);
    for (int i = 0; i < configList.count(); i++) {
        Data &config = configList[i];
        config.paletteProperty = QQmlProperty(themePalette, config.propertyName, context);

        // backup
        config.paletteBinding = QQmlPropertyPrivate::binding(config.paletteProperty);
        if (!config.paletteBinding) {
            config.paletteValue = config.paletteProperty.read();
        }

        // apply configuration
        if (config.configBinding) {
            // transfer binding's target
            if (config.configBinding->bindingType() == QQmlAbstractBinding::Binding) {
                QQmlBinding *qmlBinding = static_cast<QQmlBinding*>(config.configBinding);
                qmlBinding->setTarget(config.paletteProperty);
            }
            QQmlPropertyPrivate::setBinding(config.paletteProperty, config.configBinding);
        } else {
            if (config.paletteBinding) {
                // remove binding so the property doesn't clear it
                QQmlPropertyPrivate::setBinding(config.paletteProperty, 0);
            }
            config.paletteProperty.write(config.configProperty.read());
        }
    }
    configured = true;
}

/******************************************************************************
 * Theme
 */
UCTheme::UCTheme(QObject *parent)
    : QObject(parent)
    , m_palette(UCTheme::defaultTheme().m_palette)
    , m_engine(UCTheme::defaultTheme().m_engine)
    , m_version(UCTheme::defaultTheme().m_version)
    , m_defaultStyle(false)
{
    init();
}

UCTheme::UCTheme(bool defaultStyle, QObject *parent)
    : QObject(parent)
    , m_palette(NULL)
    , m_engine(NULL)
    , m_version(LATEST_UITK_VERSION)
    , m_defaultStyle(defaultStyle)
{
    init();
    // set the default font
    QFont defaultFont;
    defaultFont.setFamily("Ubuntu");
    defaultFont.setPixelSize(UCFontUtils::instance().sizeToPixels("medium"));
    defaultFont.setWeight(QFont::Light);
    QGuiApplication::setFont(defaultFont);
}

void UCTheme::init()
{
    m_completed = false;
    QObject::connect(&m_defaultTheme, &UCDefaultTheme::themeNameChanged,
                     this, &UCTheme::_q_defaultThemeChanged);
    updateThemePaths();
}

void UCTheme::classBegin()
{
    m_engine = qmlEngine(this);
    updateEnginePaths();
    loadPalette();
}

void UCTheme::updateEnginePaths()
{
    if (!m_engine) {
        return;
    }

    QStringList paths = themeSearchPath();
    Q_FOREACH(const QString &path, paths) {
        if (QDir(path).exists() && !m_engine->importPathList().contains(path)) {
            m_engine->addImportPath(path);
        }
    }
}

void UCTheme::_q_defaultThemeChanged()
{
    updateThemePaths();
    Q_EMIT nameChanged();
}

void UCTheme::updateThemePaths()
{
    m_themePaths.clear();

    QString themeName = name();
    while (!themeName.isEmpty()) {
        ThemeRecord themePath = pathFromThemeName(themeName);
        if (themePath.isValid()) {
            m_themePaths.append(themePath);
        }
        themeName = parentThemeName(themePath);
    }
}

/*!
 * \qmlproperty ThemeSettings ThemeSettings::parentTheme
 * \readonly
 * The property specifies the parent ThemeSettings instance.
 */
UCTheme *UCTheme::parentTheme()
{
    UCStyledItemBase *owner = qobject_cast<UCStyledItemBase*>(parent());
    UCStyledItemBasePrivate *pOwner = owner ? UCStyledItemBasePrivate::get(owner) : NULL;
    if (pOwner && pOwner->theme == this && pOwner->parentStyledItem) {
        return UCStyledItemBasePrivate::get(pOwner->parentStyledItem)->getTheme();
    }
    return NULL;
}

/*!
 * \qmlproperty string ThemeSettings::name
 * The name of the current theme in dotted format i.e. "Ubuntu.Components.Themes.Ambiance".
 */
QString UCTheme::name() const
{
    return !m_name.isEmpty() ? m_name : m_defaultTheme.themeName();
}
void UCTheme::setName(const QString& name)
{
    if (name == m_name) {
        return;
    }
    m_name = name;
    if (name.isEmpty()) {
        init();
    } else {
        QObject::disconnect(&m_defaultTheme, &UCDefaultTheme::themeNameChanged,
                            this, &UCTheme::_q_defaultThemeChanged);
        updateThemePaths();
    }
    loadPalette();
    Q_EMIT nameChanged();
}
void UCTheme::resetName()
{
    setName(QString());
}

/*!
 * \qmlproperty Palette ThemeSettings::palette
 * The palette of the current theme. When set, only the valid palette values will
 * be taken into account, which will override the theme defined palette values.
 * The following example will set the system's default theme palette normal background
 * color to Ubuntu blue. All other palette values will be untouched.
 * \qml
 * import QtQuick 2.4
 * import Ubuntu.Components 1.3
 * import Ubuntu.Components.Themes 1.0
 *
 * MainView {
 *     // your code
 *     theme.palette: Palette {
 *         normal.background: UbuntuColors.blue
 *     }
 * }
 * \endqml
 * \note Palette values applied on inherited themes will be rolled back once the
 * component declaring the palette is unloaded. This can be demonstracted using
 * a Loader element:
 * \qml
 * import QtQuick 2.4
 * import Ubuntu.Components 1.3
 * import Ubuntu.Components.Themes 1.0
 *
 * MainView {
 *     width: units.gu(40)
 *     height: units.gu(71)
 *
 *     Loader {
 *         id: loader
 *         onItemChanged: if (item) button.theme.palette = item
 *     }
 *     Component {
 *         id: dynamicPalette
 *         Palette {
 *             normal.background: UbuntuColors.blue
 *         }
 *     }
 *     Button {
 *         id: button
 *         text: "Toggle palette"
 *         onClicked: {
 *             if (loader.item) {
 *                 loader.sourceComponent = undefined;
 *             } else {
 *                 loader.sourceComponent = dynamicPalette;
 *             }
 *         }
 *     }
 * }
 * \endqml
 * The palette doesn't need to be reset as it automatically resets when the
 * palette used for configuration is destroyed.
 */
QObject* UCTheme::palette()
{
    if (!m_palette) {
        loadPalette(false);
    }
    return m_palette;
}
void UCTheme::setPalette(QObject *config)
{
    if (config == m_palette || config == m_config.palette) {
        return;
    }
    if (config && !QuickUtils::inherits(config, "Palette")) {
        qmlInfo(config) << UbuntuI18n::instance().tr("Not a Palette component.");
        return;
    }

    // 1. restore original palette values
    m_config.restorePalette();
    // 2. clear config list
    m_config.reset();
    // disconnect the reset from the previous palette
    if (m_config.palette) {
        disconnect(m_config.palette, &QObject::destroyed,
                   this, 0);
    }
    // 3. apply palette configuration
    m_config.palette = config;
    if (m_config.palette) {
        connect(m_config.palette, &QObject::destroyed,
                this, &UCTheme::resetPalette,
                Qt::DirectConnection);
        m_config.configurePalette(m_palette);
    }
    Q_EMIT paletteChanged();
}
void UCTheme::resetPalette()
{
    setPalette(NULL);
}

QUrl UCTheme::styleUrl(const QString& styleName, quint16 version, bool *isFallback)
{
    if (isFallback) {
        (*isFallback) = false;
    }
    Q_FOREACH (const ThemeRecord &themePath, m_themePaths) {
        QUrl styleUrl;
        /*
         * There are two cases where we have to deal with non-versioned styles: application
         * themes made for the previous theming and deprecated themes. For shared themes,
         * we have to check the fallback case.
         */
        quint16 styleVersion = version;
        if (themePath.deprecated) {
            styleVersion = 0;
        }
        if (themePath.shared && (version < BUILD_VERSION(1, 2))) {
            styleVersion = LATEST_UITK_VERSION;
        }

        // loop through the versions to see if we have one matching
        // we stop at version 1.2 as we do not have support for earlier themes anymore.
        for (int minor = MINOR_VERSION(styleVersion); minor >= 2; minor--) {
            QString versionedName = QStringLiteral("%1.%2/%3").arg(MAJOR_VERSION(styleVersion)).arg(minor).arg(styleName);
            styleUrl = themePath.path.resolved(versionedName);
            if (styleUrl.isValid() && QFile::exists(styleUrl.toLocalFile())) {
                // set fallback warning if the theme is shared
                if (isFallback && themePath.shared && (version != styleVersion)) {
                    (*isFallback) = true;
                }
                return styleUrl;
            }
        }

        // if we don't get any style, get the non-versioned ones for non-shared and deprecated styles
        if (!themePath.shared || themePath.deprecated) {
            styleUrl = themePath.path.resolved(styleName);
            if (styleUrl.isValid() && QFile::exists(styleUrl.toLocalFile())) {
                return styleUrl;
            }
        }
    }

    return QUrl();
}

// registers the default theme property to the root context
void UCTheme::registerToContext(QQmlContext* context)
{
    UCTheme *defaultTheme = &UCTheme::defaultTheme();
    defaultTheme->m_engine = context->engine();
    defaultTheme->updateEnginePaths();

    context->setContextProperty("theme", defaultTheme);
    ContextPropertyChangeListener *listener =
        new ContextPropertyChangeListener(context, "theme");
    QObject::connect(defaultTheme, &UCTheme::nameChanged,
                     listener, &ContextPropertyChangeListener::updateContextProperty);
}

/*!
 * \qmlproperty uint16 ThemeSettings::version
 * \since Ubuntu.Components 1.3
 * The property specifies the version of the toolkit the component is declared.
 * This equivalent with the toolkit version the component document imports. Themes,
 * starting of version 1.3, should follow the same versioning as the toolkit does.
 * If a component's style is not found under the given version, styling will try
 * to locate the style with a lower minor version until it finds a match.
 *
 * The current version of an imported toolkit module is reported by the
 * \l Ubuntu::toolkitVersion property. If a document imports Ubuntu.Components 1.2,
 * the components will load the system or application themes associated to that
 * version, and \l Ubuntu::toolkitVersion will report that version. If the document
 * imports 1.3 version, the components will load 1.3 themes. Setting this property
 * will initiate a full theme reload.
 *
 * Usually developers do not need to set this property on toolkit components as
 * those already set the version. However themes provided by applications should
 * take care of versioning the styles and on how to do theming.
 *
 * \sa Ubuntu::toolkitVersion, Ubuntu::version, {Themes}
 */
void UCTheme::setVersion(quint16 version)
{
    if (m_version == version) {
        return;
    }
    m_version = version;
    Q_EMIT versionChanged();
    // emit also nameChanged() so we reload the theme/style
    Q_EMIT nameChanged();
}

/*!
 * \qmlmethod Component ThemeSettings::createStyleComponent(string styleName, object parent)
 * Returns an instance of the style component named \a styleName and parented
 * to \a parent.
 */
QQmlComponent* UCTheme::createStyleComponent(const QString& styleName, QObject* parent)
{
    return createStyleComponent(styleName, parent, m_version);
}

QQmlComponent* UCTheme::createStyleComponent(const QString& styleName, QObject* parent, quint16 version)
{
    QQmlComponent *component = NULL;

    if (parent != NULL) {
        QQmlEngine* engine = qmlEngine(parent);
        if (engine != m_engine && !m_engine) {
            m_engine = engine;
            updateEnginePaths();
        }
        // make sure we have the paths
        if (engine != NULL) {
            bool fallback = false;
            QUrl url = styleUrl(styleName, version, &fallback);
            if (url.isValid()) {
                if (fallback) {
                    qmlInfo(parent) << QStringLiteral("Theme '%1' has no '%2' style for version %3.%4, fall back to version %5.%6.")
                                       .arg(name()).arg(styleName).arg(MAJOR_VERSION(version)).arg(MINOR_VERSION(version))
                                       .arg(MAJOR_VERSION(LATEST_UITK_VERSION)).arg(MINOR_VERSION(LATEST_UITK_VERSION));
                }
                component = new QQmlComponent(engine, url, QQmlComponent::PreferSynchronous, parent);
                if (component->isError()) {
                    qmlInfo(parent) << component->errorString();
                    delete component;
                    component = NULL;
                }
            } else {
                qmlInfo(parent) <<
                   UbuntuI18n::instance().tr(QString("Warning: Style %1 not found in theme %2").arg(styleName).arg(name()));
            }
        }
    }

    return component;
}

void UCTheme::loadPalette(bool notify)
{
    if (!m_engine) {
        return;
    }
    if (m_palette) {
        // restore bindings to the config palette before we delete
        m_config.restorePalette();
        delete m_palette;
        m_palette = 0;
    }
    // theme may not have palette defined
    QUrl paletteUrl = styleUrl("Palette.qml", m_version);
    if (paletteUrl.isValid()) {
        m_palette = QuickUtils::instance().createQmlObject(paletteUrl, m_engine);
        if (m_palette) {
            m_palette->setParent(this);
        }
        m_config.configurePalette(m_palette);
        if (notify) {
            Q_EMIT paletteChanged();
        }
    } else {
        // use the default palette if none defined
        m_palette = defaultTheme().m_palette;
    }
}

// returns the palette color value of a color profile
QColor UCTheme::getPaletteColor(const char *profile, const char *color)
{
    QColor result;
    if (palette()) {
        QObject *paletteProfile = m_palette->property(profile).value<QObject*>();
        if (paletteProfile) {
            result = paletteProfile->property(color).value<QColor>();
        }
    }
    return result;
}
