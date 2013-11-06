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
import Ubuntu.Components 0.1 as Toolkit
import Ubuntu.Components.ListItems 0.1 as ListItem

Template {
    ListItemsSection {
        title: i18n.tr("Standard")
        className: "Standard"
        delegate: ListItem.Standard {
            text: i18n.tr("Label")
        }
    }

    ListItemsSection {
        title: i18n.tr("Subtitled")
        className: "Subtitled"
        delegate: ListItem.Subtitled {
            text: i18n.tr("Label")
            subText: i18n.tr("Secondary label")

        }
    }

    ListItemsSection {
        title: i18n.tr("Icon")
        className: "Standard"
        delegate: ListItem.Standard {
            text: i18n.tr("Label")
            icon: Qt.resolvedUrl("avatar_contacts_list.png")
        }
    }

    ListItemsSection {
        title: i18n.tr("Progression")
        className: "Standard"
        delegate: ListItem.Standard {
            text: i18n.tr("Label")
            progression: true
        }
    }

    ListItemsSection {
        title: i18n.tr("Single value")
        className: "SingleValue"
        delegate: ListItem.SingleValue {
            text: i18n.tr("Label")
            value: i18n.tr("Value")
        }
    }

    ListItemsSection {
        title: i18n.tr("Multi value")
        className: "MultiValue"
        delegate: ListItem.MultiValue {
            text: i18n.tr("Label")
            values: [i18n.tr("Value 1"),
                     i18n.tr("Value 2"),
                     i18n.tr("Value 3"),
                     i18n.tr("Value 4")]
        }
    }

    ListItemsSection {
        title: i18n.tr("Item selector")
        className: "ItemSelector"

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: units.gu(3)

            ListItem.ItemSelector {
                text: i18n.tr("Expanding")
                model: [i18n.tr("Value 1"),
                        i18n.tr("Value 2"),
                        i18n.tr("Value 3"),
                        i18n.tr("Value 4")]
            }

            ListItem.ItemSelector {
                text: i18n.tr("Expanded")
                alwaysExpanded: true
                model: [i18n.tr("Value 1"),
                        i18n.tr("Value 2"),
                        i18n.tr("Value 3"),
                        i18n.tr("Value 4")]
            }

            ListItem.ItemSelector {
                text: i18n.tr("Multiple Selection")
                alwaysExpanded: false
                multiSelection: true
                model: [i18n.tr("Value 1"),
                        i18n.tr("Value 2"),
                        i18n.tr("Value 3"),
                        i18n.tr("Value 4")]
            }

            ListItem.ItemSelector {
                text: i18n.tr("Custom Model")
                model: customModel
                alwaysExpanded: true
                colourImage: true
                delegate: selectorDelegate
            }

            Component {
                id: selectorDelegate
                Toolkit.OptionSelectorDelegate { text: name; subText: description; icon: image }
            }

            ListModel {
                id: customModel
                ListElement { name: "Name 1"; description: "Description 1"; image: "images.png" }
                ListElement { name: "Name 2"; description: "Description 2"; image: "images.png" }
                ListElement { name: "Name 3"; description: "Description 3"; image: "images.png" }
                ListElement { name: "Name 4"; description: "Description 4"; image: "images.png" }
            }

            ListItem.ItemSelector {
                text: i18n.tr("Label")
                model: [i18n.tr("Value 1"),
                        i18n.tr("Value 2"),
                        i18n.tr("Value 3"),
                        i18n.tr("Value 4"),
                        i18n.tr("Value 5"),
                        i18n.tr("Value 6"),
                        i18n.tr("Value 7"),
                        i18n.tr("Value 8")]
                containerHeight: itemHeight * 4
            }

            ListItem.ItemSelector {
                text: i18n.tr("Label")
                alwaysExpanded: true
                selectedIndex: -1
                model: [i18n.tr("Value 1"),
                        i18n.tr("Value 2"),
                        i18n.tr("Value 3"),
                        i18n.tr("Value 4"),
                        i18n.tr("Value 5"),
                        i18n.tr("Value 6"),
                        i18n.tr("Value 7"),
                        i18n.tr("Value 8")]
                containerHeight: itemHeight * 4
            }
        }
    }

    ListItemsSection {
        title: i18n.tr("Control")
        className: "Standard"
        delegate: ListItem.Standard {
            text: i18n.tr("Label")
            control: Toolkit.Switch {
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    ListItemsSection {
        title: i18n.tr("Removable")
        className: "Standard"
        delegate: ListItem.Standard {
            text: i18n.tr("Slide to remove")
            removable: true
            confirmRemoval: (index % 2)
            backgroundIndicator: Rectangle {
                anchors.fill: parent
                color: Theme.palette.normal.base
            }
        }
    }

    TemplateSection {
        title: i18n.tr("Grouped list")
        className: "Header"

        Rectangle {
            color: Qt.rgba(0.0, 0.0, 0.0, 0.01)
            anchors.left: parent.left
            anchors.right: parent.right
            height: groupedList.height

            ListModel {
                id: groupedModel
                ListElement { name: "Orange"; type: "Fruit"}
                ListElement { name: "Apple"; type: "Fruit" }
                ListElement { name: "Tomato"; type: "Fruit" }
                ListElement { name: "Carrot"; type: "Vegetable" }
                ListElement { name: "Potato"; type: "Vegetable" }
            }

            ListView {
                id: groupedList
                model: groupedModel
                width: parent.width
                height: contentHeight
                interactive: false
                delegate: ListItem.Standard {
                    text: i18n.tr(name)
                }
                section.property: "type"
                section.criteria: ViewSection.FullString
                section.delegate: ListItem.Header { text: i18n.tr(section) }
            }
        }
    }
}
