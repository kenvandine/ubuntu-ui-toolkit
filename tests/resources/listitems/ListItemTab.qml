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

import QtQuick 2.3
import Ubuntu.Components 1.2
import Ubuntu.Components.ListItems 1.0
import QtQuick.Layouts 1.1

Tab {
    title: "ListItem with no layout"
    ListItemActions {
        id: sharedLeading
        actions: Action {
            iconName: "delete"
            text: "Delete"
        }
    }
    ListItemActions {
        id: sharedTrailing
        actions: [
            Action {
                iconName: "search"
                text: "Search"
            },
            Action {
                iconName: "edit"
                text: "Edit"
            },
            Action {
                iconName: "email"
                text: "E-mail"
            }
        ]
    }

    Action {
        id: defaultAction
        onTriggered: {
            // open a dialog
        }
    }
    page: Page {
        Flickable {
            anchors.fill: parent
            contentHeight: column.childrenRect.height
            Column {
                id: column
                width: parent.width

                Header { text: "Highlight policy demo" }
                ListItem {
                    Label {
                        text: "No action, leading/trailing actions, or active component added." +
                              " Tapping on it will not produce highligh when <b>ListItem.AutomaticHighlight</b> is set."
                        wrapMode: Text.Wrap
                        width: parent.width
                    }
                }
                ListItem {
                    highlightPolicy: ListItem.PermanentHighlight
                    Label {
                        text: "No action, leading/trailing actions, or active component added." +
                              " Tapping on it will do highligh when <b>ListItem.PermanentHighlight</b> is set."
                        wrapMode: Text.Wrap
                        width: parent.width
                    }
                }
                ListItem {
                    Row {
                        width: parent.width
                        CheckBox {
                            id: check
                        }
                        Label {
                            text: "An active component will allow highligh when not pressed over the active component, "+
                                   "if <b>ListItem.AutomaticHighlight</b> is set."
                            wrapMode: Text.Wrap
                            width: parent.width - check.width
                        }
                    }
                }
                ListItem {
                    highlightPolicy: ListItem.PermanentHighlight
                    Row {
                        width: parent.width
                        CheckBox {
                            id: check2
                        }
                        Label {
                            text: "When clicked over an active component, the entire ListItem will be highlight, "+
                                   "if <b>ListItem.PermanentHighlight</b> is set."
                            wrapMode: Text.Wrap
                            width: parent.width - check2.width
                        }
                    }
                }
            }
        }
    }
}


