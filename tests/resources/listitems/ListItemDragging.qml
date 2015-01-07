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

import QtQuick 2.2
import Ubuntu.Components 1.2

MainView {
    id: main
    width: units.gu(40)
    height: units.gu(60)
    useDeprecatedToolbar: false

    Page {
        title: "Dragging test"
        ListView {
            anchors.fill: parent
            ListItem.selectMode: ListItem.dragMode
            contentItem.objectName: "ListViewContent"

            model: ListModel {
                Component.onCompleted: {
                    for (var i = 0; i < 25; i++) {
                        append({label: "List item #"+i})
                    }
                }
            }

            delegate: ListItem {
                objectName: "ListItem-" + index
                leadingActions: ListItemActions {
                    actions: Action {
                        iconName: "delete"
                    }
                }

                Label {
                    text: label
                }

                onPressAndHold: {
                    print("entering/leaving draggable mode")
                    ListView.view.ListItem.dragMode = !ListView.view.ListItem.dragMode;
                }
            }
        }
    }
}