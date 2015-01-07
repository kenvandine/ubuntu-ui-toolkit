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
import Ubuntu.Components 1.0
import Ubuntu.Components.ListItems 0.1 as ListItem
import Ubuntu.Components.Popups 1.0
import Qt.labs.folderlistmodel 2.1

Template {
    objectName: "iconsTemplate"

    TemplateSection {
        className: "Icon"

        TemplateRow {
            title: i18n.tr("Scaling")
            spacing: units.gu(2)

            Icon {
                name: "call-start"
                width: 16
                height: 16
                anchors.verticalCenter: parent.verticalCenter
            }

            Icon {
                name: "call-start"
                width: 48
                height: 48
                anchors.verticalCenter: parent.verticalCenter
            }

            Icon {
                name: "call-start"
                width: 96
                height: 96
            }
        }

        TemplateRow {
            title: i18n.tr("Colorization")
            spacing: units.gu(2)

            Icon {
                name: "stock_alarm-clock"
                width: 24
                height: 24
            }

            Icon {
                name: "stock_alarm-clock"
                color: UbuntuColors.orange
                width: 24
                height: 24
            }

            Icon {
                name: "stock_alarm-clock"
                color: UbuntuColors.lightAubergine
                width: 24
                height: 24
            }
        }

        TemplateRow {
            title: i18n.tr("Theme")
            spacing: units.gu(2)
            height: iconFlow.height

            Flow {
                id: iconFlow
                width: parent.width
                spacing: units.gu(2)

                Repeater {
                    model: FolderListModel {
                        folder: "/usr/share/icons/suru/actions/scalable"
                        showDirs: false
                        showOnlyReadable : true
                        sortField: FolderListModel.Name
                        nameFilters: [ "*.svg" ]
                    }
                    Icon {
                        id: themedIcon
                        name: fileBaseName || ""
                        width: 24
                        height: 24
                        MouseArea {
                            anchors.fill: parent
                            onClicked: PopupUtils.open(iconPop, themedIcon, { 'icon': themedIcon.name })
                        }
                        Component {
                            id: iconPop
                            Popover {
                                id: iconPopover
                                property string icon: "N/A"

                                Column {
                                    anchors.top: parent.top
                                    anchors.left: parent.left
                                    anchors.right: parent.right

                                    ListItem.Standard {
                                        iconName: iconPopover.icon
                                        text: iconPopover.icon
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
