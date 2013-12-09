/*
 * Copyright 2012 Canonical Ltd.
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
import Ubuntu.Components 0.1

Item {
    id: headerStyle
    /*!
      The height of the headercontents, which is the full height of
      the header minus the separators shown at the bottom of it.
     */
    property real contentHeight: units.gu(7.5)

    /*!
      The source of the image that separates the header from the contents of a \l MainView.
      The separator will be drawn over the contents.
     */
    property url separatorSource: "artwork/PageHeaderBaseDividerLight.sci"

    /*!
      The source of an additional image attached to the bottom of the separator. The contents
      of the \l MainView will be drawn on top of the separator bottom image.
     */
    property url separatorBottomSource: "artwork/PageHeaderBaseDividerBottom.png"

    property int fontWeight: Font.Light
    property string fontSize: "x-large"
    property color textColor: Theme.palette.selected.backgroundText
    property real textLeftMargin: styledItem.controls ? 0 : units.gu(2)

    implicitHeight: headerStyle.contentHeight + separator.height + separatorBottom.height

    BorderImage {
        id: separator
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        source: headerStyle.separatorSource
    }
    Image {
        id: separatorBottom
        anchors {
            top: separator.bottom
            left: parent.left
            right: parent.right
        }
        source: headerStyle.separatorBottomSource
    }

    Item {
        id: controlContainer
        anchors {
            left: parent.left
            top: parent.top
        }
        height: headerStyle.contentHeight
        width: styledItem.controls ? styledItem.controls.width : 0

        Binding {
            target: styledItem.controls
            property: "parent"
            value: controlContainer
            when: styledItem.controls
        }

        // Compensate for the lack of a MouseArea in the ToolbarButton by
        //  adding a MouseArea on top of it.
        MouseArea {
            // FIXME: Currently this only works if the controls is a single
            //  ToolbarButton. To support multiple controls, use a similar
            //  detection of the clicked item as in Panel.qml.
            anchors.fill: parent
            onClicked: {
                styledItem.controls.trigger();
            }
            visible: styledItem.controls && styledItem.controls.hasOwnProperty("trigger") &&
                     !styledItem.controls.hasOwnProperty("clicked")
        }
    }

    Item {
        id: foreground
        anchors {
            left: controlContainer.right
            right: parent.right
            top: parent.top
        }
        height: headerStyle.contentHeight

        Label {
            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
                leftMargin: headerStyle.textLeftMargin
            }
            text: styledItem.title
            font.weight: headerStyle.fontWeight
            visible: !styledItem.contents
            fontSize: headerStyle.fontSize
            color: headerStyle.textColor
        }

        Binding {
            target: styledItem.contents
            property: "anchors.fill"
            value: foreground
            when: styledItem.contents
        }
        Binding {
            target: styledItem.contents
            property: "parent"
            value: foreground
            when: styledItem.contents
        }
    }
}
