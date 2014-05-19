/*
 * Copyright (C) 2014 Canonical Ltd.
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
import Ubuntu.Components 1.1

MainView {
    width: units.gu(50)
    height: units.gu(80)
    useDeprecatedToolbar: false

    Page {
        title: "test page"

        id: page

        __customHeaderContents: Item {
            TextField {
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }
            }
        }

        Label {
            anchors.centerIn: parent
            text: "Hello, world"
        }

        tools: ToolbarItems {
            ToolbarButton {
                action: Action {
                    iconName: "contact"
                    text: "oh"
                    onTriggered: print("lala")
                    enabled: false
                }
            }

            back: ToolbarButton {
                action: Action {
                    text: "cancel"
                    iconName: "cancel"
                    onTriggered: {
                        page.__customHeaderContents = null;
                    }
                }
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
