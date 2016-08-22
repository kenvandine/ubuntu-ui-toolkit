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

#include "ucaction_p.h"
#include "quickutils_p.h"
#include "ucactioncontext_p.h"

#include <QtDebug>
#include <QtQml/QQmlInfo>
#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickwindow.h>
#include <private/qguiapplication_p.h>

Q_LOGGING_CATEGORY(ucAction, "ubuntu.components.Action", QtMsgType::QtWarningMsg)

#define ACT_TRACE(params) qCDebug(ucAction) << params

UT_NAMESPACE_BEGIN

bool shortcutContextMatcher(QObject* object, Qt::ShortcutContext context)
{
    UCAction* action = static_cast<UCAction*>(object);
    if (!action->isEnabled()) {
        return false;
    }

    switch (context) {
    case Qt::ApplicationShortcut:
        return true;
    case Qt::WindowShortcut: {
        QObject* window = object;
        while (window && !window->isWindowType()) {
            window = window->parent();
            if (::QQuickItem* item = qobject_cast<::QQuickItem*>(window)) {
                window = item->window();
            }
        }
        bool activatable = window && window == QGuiApplication::focusWindow();

        if (activatable) {
            // is the last action owner item in an active context?
            QQuickItem *pl = action->lastOwningItem();
            activatable = false;
            while (pl) {
                UCActionContextAttached *attached = static_cast<UCActionContextAttached*>(
                            qmlAttachedPropertiesObject<UCActionContext>(pl, false));
                if (attached) {
                    activatable = attached->context()->active();
                    if (!activatable) {
                        ACT_TRACE(action << "Inactive context found" << attached->context());
                        break;
                    }
                }
                pl = pl->parentItem();
            }
            if (!activatable) {
                // check if the action is in an active context
                UCActionContext *context = qobject_cast<UCActionContext*>(action->parent());
                activatable = context && context->active();
            }
        }
        if (activatable) {
            ACT_TRACE("SELECTED ACTION" << action);
        }

        return activatable;
    }
    default: break;
    }
    return false;
}

/*!
 * \qmltype Action
 * \instantiates UCAction
 * \inqmlmodule Ubuntu.Components
 * \ingroup ubuntu
 * \brief Describe an action that can be re-used and shared between different
 * components.
 *
 * Actions can be used to define a specific task to be executed in different
 * contexts using different components. The same action can be assigned to
 * a \l Button, a \b Checkbox or even a \l TextField. The \l triggered signal
 * is emitted depending on the component. \l Button and \b CheckBox for instance
 * emits the signal when clicked, whereas \l TextField emits the signal when
 * its accepted signal is triggered.
 *
 * If the \l parameterType property is set, the Action is said to be parameterised.
 * This means that when it is bound to a menu or button, the action expects a
 * typed input parameter. The type affects the allowed value of the QVariant
 * that must be passed to the \l trigger and \l triggered.
 * \qml
 * Action {
 *     id: action
 *     parameterType: Action.Integer
 *     text: "Int value"
 *     onTriggered: {
 *         // the value will be undefined
 *         console.log("value is", value);
 *     }
 *     Component.onCompleted: trigger("Hello World!")
 * }
 * \endqml
 *
 * When an Action is assigned to a component, the component takes the values
 * from the action itself. Therefore assigning the action to a Button is enough
 * to set up the label and the icon to be shown by the button.
 * \code
 * Action {
 *     id: stock
 *     iconName: "call"
 *     text: "Call"
 * }
 * Button {
 *     // this binding will set the Button's text, iconName and
 *     // iconSource properties.
 *     action: stock
 * }
 * \endcode
 *
 * Actions are used to populate different Popovers like \l ActionSelectionPopover
 * as well as to define actions for pages, or when defining options in \c ListItemOptions.
 *
 * Examples: See \l Page
 */

/*!
 * \qmlsignal Action::triggered(var value)
 * Signal called when the action is triggered. The user visible primary label of
 * the action when emitted by components. Custom implementations must make sure
 * this rule is followed, therefore instead of emitting the signal the \l trigger
 * function should be called.
 */

/*!
 * \qmlproperty string Action::description
 * User visible secondary description for the action. Description is more verbose
 * than the \l text and should describe the Action with couple of words.
 */


/*!
 * \qmlproperty string Action::text
 * The user visible primary label of the action.
 */
QString UCAction::text()
{
    return m_text;
}
void UCAction::setText(const QString &text)
{
    if (m_text == text) {
        return;
    }
    m_text = text;
    Q_EMIT textChanged();
}
void UCAction::resetText()
{
    setText(QString());
}

/*!
 * \qmlproperty string Action::keywords
 * Additional user visible keywords for the action.
 * The format of the keywords string is "Keyword 1;Keyword 2;Keyword 3" to allow
 * translators to define different number of keywords per language. The keywords
 * are separated by ; and they may contain spaces.
 * \qml
 * Action {
 *     text: i18n.tr("Crop")
 *     description: i18n.tr("Crop the image")
 *     keywords: i18n.tr("Trim;Cut")
 * }
 * \endqml
 */

/*!
 * \qmlproperty enum Action::parameterType
 * Type of the parameter passed to \l trigger and \l triggered.
 * Type is an enumeration:
 * \list
 *  \li \b Action.None: No paramater. (default)
 *  \li \b Action.String: String parameter.
 *  \li \b Action.Integer: Integer parameter.
 *  \li \b Action.Bool: Boolean parameter.
 *  \li \b Action.Real: Single precision floating point parameter.
 *  \li \b Action.Object: The parameter is an object.
 * \endlist
 * \qml
 * Action {
 *     id: action
 *     parameterType: Action.String
 *     onTriggered: {
 *         // value arguments now contain strings
 *         console.log(value);
 *     }
 *     Component.onCompleted: action.trigger("Hello World")
 * }
 * \endqml
 */

/*!
 * \qmlproperty bool Action::enabled
 * If set to false the action can not be triggered. Components visualizing the
 * action migth either hide the action or make it insensitive. However visibility
 * can be controlled separately using the \l visible property.
 */

/*!
 * \qmlproperty bool Action::visible
 * Specifies whether the action is visible to the user. Defaults to true.
 */

UCAction::UCAction(QObject *parent)
    : QObject(parent)
    , m_itemHint(Q_NULLPTR)
    , m_parameterType(None)
    , m_factoryIconSource(true)
    , m_enabled(true)
    , m_visible(true)
    , m_published(false)
{
    generateName();
}

UCAction::~UCAction()
{
    resetShortcut();
    resetText();
}

bool UCAction::isValidType(QVariant::Type valueType)
{
    bool valid = (valueType == QVariant::String && m_parameterType == String) ||
                (valueType == QVariant::Int && m_parameterType == Integer) ||
                (valueType == QVariant::Bool && m_parameterType == Bool) ||
                (valueType == QVariant::Double && m_parameterType == Real) ||
                (valueType == QVariant::Invalid && m_parameterType == None) ||
                (valueType == (QVariant::Type)QMetaType::QObjectStar && m_parameterType == Object);
    return valid;
}

void UCAction::generateName()
{
    static int id = 0;
    m_name = QString("unity-action-%1").arg(id++);
}

/*!
 * \qmlproperty string Action::iconName
 * The icon associated with the action. If both iconName and \l iconSource are
 * defined, iconName will be ignored by the components.
 * \note The complete list of icons available in Ubuntu is not published yet.
 * For now please refer to the folder where the icon theme is installed:
 * \list
 *  \li Ubuntu Touch: \l file:/usr/share/icons/suru
 * \endlist
 */
void UCAction::setIconName(const QString &name)
{
    if (m_iconName == name) {
        return;
    }
    m_iconName = name;
    if (m_factoryIconSource) {
        m_iconSource = m_iconName.isEmpty() ? QUrl() : QUrl("image://theme/" + m_iconName);
        Q_EMIT iconSourceChanged();
    }
    Q_EMIT iconNameChanged();
}

/*!
 * \qmlproperty string Action::name
 * The name of the action. By default an action gets it's name generated automatically
 * if not overridden with later. If name is set to "" then the action restores it's
 * autogenerated name. The name is not user visible.
 */
void UCAction::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }
    m_name = name;
    if (m_name.isEmpty()) {
        generateName();
    }
    Q_EMIT nameChanged();
}

/*!
 * \qmlproperty url Action::iconSource
 * This is a URL to any image file.
 * In order to use an icon from the Ubuntu theme, use the \l iconName property instead.
 */
void UCAction::setIconSource(const QUrl &url)
{
    if (m_iconSource == url) {
        return;
    }
    m_iconSource = url;
    m_factoryIconSource = false;
    Q_EMIT iconSourceChanged();
}

/*!
 * \qmlproperty Component Action::itemHint
 * \deprecated
 * \b {itemHint is DEPRECATED. Use \l ActionItem to specify the representation
 * of an \l Action.}
 */
void UCAction::setItemHint(QQmlComponent *)
{
    qWarning() << "Action.itemHint is a DEPRECATED property. Use ActionItems to specify the representation of an Action.";
}

QKeySequence sequenceFromVariant(const QVariant& variant) {
    if (variant.type() == QVariant::Int) {
        return static_cast<QKeySequence::StandardKey>(variant.toInt());
    }
    if (variant.type() == QVariant::String) {
        return QKeySequence::fromString(variant.toString());
    }
    return QKeySequence();
}

/*!
 * \qmlproperty var Action::shortcut
 * The keyboard shortcut that can be used to trigger the action.
 * \b StandardKey values such as \b StandardKey.Copy
 * as well as strings in the form "Ctrl+C" are accepted values.
 * \since 1.3
 */
void UCAction::setShortcut(const QVariant& shortcut)
{
    if (m_shortcut.isValid())
        QGuiApplicationPrivate::instance()->shortcutMap.removeShortcut(0, this, sequenceFromVariant(m_shortcut));

    QKeySequence sequence(sequenceFromVariant(shortcut));
    if (!sequence.isEmpty()) {
        ACT_TRACE("ADD SHORTCUT" << sequence.toString());
        QGuiApplicationPrivate::instance()->shortcutMap.addShortcut(this, sequence, Qt::WindowShortcut, shortcutContextMatcher);
    } else {
        qmlInfo(this) << "Invalid shortcut: " << shortcut.toString();
    }

    m_shortcut = shortcut;
    Q_EMIT shortcutChanged();
}
void UCAction::resetShortcut()
{
    if (!m_shortcut.isValid()) {
        return;
    }
    QGuiApplicationPrivate::instance()->shortcutMap.removeShortcut(0, this, sequenceFromVariant(m_shortcut));
    m_shortcut = QVariant();
    Q_EMIT shortcutChanged();
}

bool UCAction::event(QEvent *event)
{
    if (event->type() != QEvent::Shortcut)
        return false;

    // when we reach this point, we can be sure the Action is used
    // by a component belonging to an active ActionContext.
    QShortcutEvent *shortcut_event(static_cast<QShortcutEvent*>(event));
    if (shortcut_event->isAmbiguous()) {
        qmlInfo(this) << "Ambiguous shortcut: " << shortcut_event->key().toString();
        return false;
    }

    // do not call trigger() directly but invoke, as it may get overridden in QML
    invokeTrigger<UCAction>(this, QVariant());
    return true;
}

/*!
 * \qmlmethod Action::trigger(var value)
 * Checks the \c value against the action \l parameterType and triggers the action.
 * If \l parameterType is \c Action.None, it will trigger as
 * \code
 * action.trigger()
 * \endcode
 */
void UCAction::trigger(const QVariant &value)
{
    if (!m_enabled) {
        return;
    }
    if (!isValidType(value.type())) {
        Q_EMIT triggered(QVariant());
    } else {
        Q_EMIT triggered(value);
    }
}

void UCAction::addOwningItem(QQuickItem *item)
{
    if (!m_owningItems.contains(item)) {
        m_owningItems.append(item);
        ACT_TRACE("ADD ACTION OWNER" << item->objectName() << "TO" << this);
    }
}

void UCAction::removeOwningItem(QQuickItem *item)
{
    m_owningItems.removeOne(item);
    ACT_TRACE("REMOVE ACTION OWNER" << item->objectName() << "FROM" << this);
}

UT_NAMESPACE_END
