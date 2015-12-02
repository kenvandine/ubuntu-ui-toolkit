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
 */

import QtQuick 2.4
import Ubuntu.Components 1.3

StyledItem {
    id: root

    implicitWidth: 240
    implicitHeight: 150
    activeFocusOnPress: true

    //TODO: add horizontalScrollbarPolicy
    //TODO: add verticalScrollbarPolicy

    property bool alwaysOnScrollbars: false

    /*!
        \qmlproperty Item ScrollView::viewport
        This property holds the viewport Item. The children of the ScrollView element are
        reparented to this item to make sure the scrollbars are correctly positioned and
        the items are clipped at their boundaries.
    */
    readonly property alias viewport: viewportItem

    /*!
        \qmlproperty Item ScrollView::flickableItem
        The flickableItem of the ScrollView. If the contentItem provided
        to the ScrollView is a Flickable, that will be the \l flickableItem.
        Otherwise ScrollView will create a Flickable which will hold the
        items provided as children.
    */
    readonly property alias flickableItem: internal.flickableItem

    /*!
        The contentItem of the ScrollView. This is set by the user.
        Note that the definition of contentItem is somewhat different to that
        of a Flickable, where the contentItem is implicitly created.
    */
    default property Item contentItem

    /*! \internal */
    //property alias __horizontalScrollBar: scroller.horizontalScrollBar
    /*! \internal */
    //property alias __verticalScrollBar: scroller.verticalScrollBar

    onContentItemChanged: {
        // Check if the item provided is a Flickable
        if (contentItem.hasOwnProperty("contentWidth") &&
                contentItem.hasOwnProperty("flickableDirection")) {
            internal.flickableItem = contentItem
            internal.flickableItem.parent = viewportItem
        } else {
            //Create a dummy Flickable if the app dev didn't provide any
            internal.flickableItem = flickableComponent.createObject(viewportItem)
            contentItem.parent = internal.flickableItem.contentItem
        }
        internal.flickableItem.anchors.fill = viewportItem
    }

    children: Item {
        id: internal

        property Flickable flickableItem
        property real nonOverlayScrollbarMargin: verticalScrollbar.__styleInstance ? verticalScrollbar.__styleInstance.nonOverlayScrollbarMargin : 0

        //if the flickable is not coming from the user but from our internal Component...
        Binding {
            target: flickableItem
            when: contentItem !== flickableItem
            property: "contentHeight"
            value: contentItem ? contentItem.height : 0
        }
        Binding {
            target: flickableItem
            when: contentItem !== flickableItem
            property: "contentWidth"
            value: contentItem ? contentItem.width : 0
        }

        anchors.fill: parent

        Component {
            id: flickableComponent
            Flickable {}
        }

        Item {
            id: viewportItem
            anchors.fill: parent
            anchors.topMargin: (horizontalScrollbar.align === Qt.AlignTop && horizontalScrollbar.__alwaysOnScrollbars)
                               ? internal.nonOverlayScrollbarMargin : 0
            anchors.leftMargin: (verticalScrollbar.align === Qt.AlignLeading && horizontalScrollbar.__alwaysOnScrollbars)
                                ? internal.nonOverlayScrollbarMargin : 0
            anchors.rightMargin: (verticalScrollbar.align === Qt.AlignTrailing && horizontalScrollbar.__alwaysOnScrollbars)
                                 ? internal.nonOverlayScrollbarMargin : 0
            anchors.bottomMargin: (horizontalScrollbar.align === Qt.AlignBottom && horizontalScrollbar.__alwaysOnScrollbars)
                                  ? internal.nonOverlayScrollbarMargin : 0

            //shortScrollingRation is used for arrow keys, longScrollingRatio is used for pgUp/pgDown
            //0.1 means we will scroll 10% of the *visible* flickable area
            property real shortScrollingRatio: __styleInstance ? __styleInstance.shortScrollingRatio : 0.1
            property real longScrollingRatio: __styleInstance ? __styleInstance.longScrollingRatio : 0.9

            clip: true
            focus: true
            Keys.enabled: true
            Keys.onLeftPressed: {
                console.log("Left pressed")
                if (horizontalScrollbar.__styleInstance !== null) {
                    horizontalScrollbar.__styleInstance.scroll(-flickableItem.width*shortScrollingRatio)
                }
            }
            Keys.onRightPressed: {
                console.log("Right pressed")
                if (horizontalScrollbar.__styleInstance !== null) {
                    horizontalScrollbar.__styleInstance.scroll(flickableItem.width*shortScrollingRatio)
                }
            }
            Keys.onDownPressed: {
                console.log("Down pressed")
                if (verticalScrollbar.__styleInstance !== null) {
                    verticalScrollbar.__styleInstance.scroll(flickableItem.height*shortScrollingRatio)
                }
            }
            Keys.onUpPressed: {
                console.log("Up pressed")
                if (verticalScrollbar.__styleInstance !== null) {
                    verticalScrollbar.__styleInstance.scroll(-flickableItem.height*shortScrollingRatio)
                }
            }
            Keys.onPressed:  {
                console.log("Pressed")
                if (event.key == Qt.Key_Escape) {
                    var scrollbarWithActiveDrag = (horizontalScrollbar.__styleInstance && horizontalScrollbar.__styleInstance.draggingThumb)
                            || (verticalScrollbar.__styleInstance && verticalScrollbar.__styleInstance.draggingThumb)
                            || null
                    if (scrollbarWithActiveDrag !== null) {
                        scrollbarWithActiveDrag.__styleInstance.resetScrollingToPreDrag()
                    }
                    event.accepted = true
                } else if (verticalScrollbar.__styleInstance !== null) {
                    if (event.key == Qt.Key_PageDown) {
                        verticalScrollbar.__styleInstance.scroll(flickableItem.height*longScrollingRatio)
                    } else if (event.key == Qt.Key_PageUp) {
                        verticalScrollbar.__styleInstance.scroll(-flickableItem.height*longScrollingRatio)
                    } else if (event.key == Qt.Key_Home) {
                        verticalScrollbar.__styleInstance.scrollToBeginning()
                    } else if (event.key == Qt.Key_End) {
                        verticalScrollbar.__styleInstance.scrollToEnd()
                    }
                    event.accepted = true
                }
            }
        }

        //When you click outside of a child of the scrollview, this restores the focus to the ScrollView
        //Why is this needed?
        //  Suppose the viewport has a child ScrollView (or another Item which handles some or all of the hw keys we handle).
        //  Now, if that child is coded to set its focus to true when you tap on it (which is, for instance, what
        //  StyledItem's activeFocusOnPress does), tapping on that child will set viewportItem.focus = false, because
        //  that child is just another Item inside "root" and "root" is a FocusScope, and that's how FocusScope works.
        //  At this point, we need something to restore the focus to the viewportItem once you tap on viewportItem, outside
        //  any other child. And that's what the MouseArea below does.
        //
        //The alternative could be making the viewport a StyledItem and using activeFocusOnPress:true, but that wouldn't work
        //because it would make viewportItem a focus scope, and we don't want that, because at that point,
        //being in a similar situation as described above, we wouldn't be able to transfer the focus to viewportItem, because
        //being a focus scope means viewportItem would forward the focus to the one between its children who asked it most
        //recently
        MouseArea {
            anchors.fill: parent
            enabled: true
            onPressed: {
                console.log("REQUESTING FOCUS ON VIEWPORT OF", root)
                viewportItem.focus = true
                mouse.accepted = false
            }
        }
        Scrollbar {
            id: horizontalScrollbar
            flickableItem: internal.flickableItem
            __viewport: viewportItem
            align: Qt.AlignBottom
            buddyScrollbar: verticalScrollbar
            __alwaysOnScrollbars: alwaysOnScrollbars
            focus: false
        }

        Scrollbar {
            id: verticalScrollbar
            flickableItem: internal.flickableItem
            __viewport: viewportItem
            align: Qt.AlignTrailing
            buddyScrollbar: horizontalScrollbar
            __alwaysOnScrollbars: alwaysOnScrollbars
            focus: false
        }

        Column {
            anchors.left: viewportItem.left
            anchors.right: viewportItem.right
            Text { color: root.activeFocus ? "red" : "black"; text:"ROOT focus " + root.focus + " activeFocus " + root.activeFocus; }
            Text { color: viewportItem.activeFocus ? "red" : "black"; text:"VIEWPORT focus " + viewportItem.focus + " activeFocus " + viewportItem.activeFocus; }
            Text { color: internal.activeFocus ? "red" : "black"; text:"INTERNAL focus " + internal.focus + " activeFocus " + internal.activeFocus; }
        }

    }
}