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

/*
  This component is the holder of the ListItem options.
  */
Item {
    id: panel
    width: optionsRow.childrenRect.width

    // for testing
    objectName: "ListItemPanel"

    /*
      Property holding the ListItem's contentItem instance
      */
    readonly property Item contentItem: parent ? parent.contentItem : null

    /*
      Index of the ListItem, if the ListItem is inside a ListView or has been
      created using a Repeater.
      */
    property int listItemIndex

    /*
      Specifies whether the panel is used to visualize leading or trailing options.
      */
    property bool leadingPanel: ListItemActions.status == ListItemActions.Leading

    /*
      Actions
      */
    property var actionList: ListItemActions.container ? ListItemActions.container.actions : undefined

    // fire selected action when disconnected
    onParentChanged: {
        if (!parent && selectedAction) {
            selectedAction.triggered(listItemIndex >= 0 ? listItemIndex : null);
            selectedAction = null;
        }
    }
    property Action selectedAction: null

    anchors {
        left: contentItem ? (leadingPanel ? undefined : contentItem.right) : undefined
        right: contentItem ? (leadingPanel ? contentItem.left : undefined) : undefined
        top: contentItem ? contentItem.top : undefined
        bottom: contentItem ? contentItem.bottom : undefined
    }

    Rectangle {
        objectName: "panel_background"
        anchors.fill: parent
        // FIXME: use Palette colors instead when available
        color: (panel.ListItemActions.container.backgroundColor != "#000000") ?
                   panel.ListItemActions.container.backgroundColor : (leadingPanel ? UbuntuColors.red : "white")
    }

    Row {
        id: optionsRow
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            leftMargin: spacing
        }

        property real maxItemWidth: panel.parent ? (panel.parent.width / panel.actionList.length) : 0

        Repeater {
            model: panel.actionList
            AbstractButton {
                action: modelData
                visible: action.visible && action.enabled
                width: (!visible || !enabled) ?
                           0 : MathUtils.clamp(delegateLoader.item ? delegateLoader.item.width : 0, height, optionsRow.maxItemWidth)
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                }

                function trigger() {
                    // save the action as we trigger when the rebound animation is over
                    // to make sure we properly clean up the blockade of the Flickables
                    panel.selectedAction = action;
                    panel.listItemIndex = panel.ListItemActions.listItemIndex;
                    panel.ListItemActions.snapToPosition(0.0);
                }

                Loader {
                    objectName: "icon_loader"
                    id: delegateLoader
                    height: parent.height
                    sourceComponent: (panel.ListItemActions.container && panel.ListItemActions.container.delegate) ?
                                         panel.ListItemActions.container.delegate : defaultDelegate
                    property Action action: modelData
                    property int index: index
                    onItemChanged: {
                        // this is needed only for testing purposes
                        if (item && item.objectName === "") {
                            item.objectName = "list_option_" + index
                        }
                    }
                }
            }
        }
    }

    Component {
        id: defaultDelegate
        Item {
            width: height
            Icon {
                objectName: "action_icon"
                width: units.gu(2.5)
                height: width
                name: action.iconName
                // FIXME: use Palette colors instead when available
                color: (panel.ListItemActions.container.foregroundColor != "#000000") ?
                           panel.ListItemActions.container.foregroundColor : (panel.leadingPanel ? "white" : UbuntuColors.darkGrey)
                anchors.centerIn: parent
            }
        }
    }
}
