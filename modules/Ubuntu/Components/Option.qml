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
 */

import QtQuick 2.0
import U1db 1.0 as U1db

/*!
    \qmltype Option
    \inqmlmodule Ubuntu.Components 0.1
    \ingroup ubuntu
    \brief An option within application-specific settings.

    Example:
    \qml
    Settings {
        group "sound
        Option {
            name: "vibrate"
            defaultValue: false
        }
        Option {
            id: optionNickname
            name: "nomDePlum"
            defaultValue: "A. Nonymous"
            text: i18n.tr("Pseudonym")
            visible: proFeaturesUnlocked // variable defined in the application
        }
    }

    Column {
        Row {
            Label {
                text: optionVibrate.text
            }
            Switch {
                action: optionVibrate
                text: optionVibrate.value
            }
        }
        Row {
            Label {
                text: optionNickname.text
            }
            TextField {
                action: optionNickname
                text: optionNickname.value
            }
        }
    }
    \endqml

    See Settings for more examples.
*/
Action {
    id: option
    /*!
      A name for the setting that is unique in the group.
      */
    property string name: null
    /*!
      The default value. Any basic type is allowed. After the first time an
      option is used on one particular device, the default is ignored unless
      the group is reset.
      */
    property var defaultValue: null
    /*!
      The current value, either a default, locally saved state or synchronized.
      */
    property var value: defaultValue
    onValueChanged: {
        if (__doc) {
            var ct = __doc.contents
            ct[name] = value
            __doc.contents = ct
        }
    }
    /*!
      \internal
      The document to propagate changes to.
      */
    property var __doc: null

    onDefaultValueChanged: {
        /* FIXME: UnityActions.Action.Type.String doesn't work */
        if (typeof defaultValue == "string")
            parameterType = 1
        else if (typeof defaultValue == "number")
            parameterType = 4 // Javascript doesn't distinguish int (2) and real (4)
        else if (typeof defaultValue == "boolean")
            parameterType = 3
        else
            parameterType = 0
    }

    onTriggered: {
        option.value = value
    }
}
