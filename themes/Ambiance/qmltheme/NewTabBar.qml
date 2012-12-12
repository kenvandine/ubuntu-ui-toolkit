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
    id: tabBar
    height: itemStyle.tabBarHeight

    /*!
      The set of tabs this tab bar belongs to
     */
    property Tabs tabs

    /*!
      An inactive tab bar only displays the currently selected tab,
      and an active tab bar can be interacted with to select a tab.
     */
    property bool active: false

    onActiveChanged: {
//        indicatorImage.visible = false;
        if (active) {
            activatingTimer.restart();
        } else {
            buttonView.selectButton(tabs.selectedTabIndex);
        }
    }

    /*!
      \internal
      Avoid interpreting a click to activate the tab bar as a button click.
     */
    Timer {
        id: activatingTimer
        interval: 800 // same as pressAndHold time
    }

    Connections {
        target: tabs
        onSelectedTabIndexChanged: {
//            indicatorImage.visible = false;
            buttonView.selectButton(tabs.selectedTabIndex);
        }
    }

    PathView {
        id: buttonView
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }

        // used to position buttons and indicator image
        property real buttonRowWidth: 1
        property var buttons: []

        // Track which button was last clicked
        property int selectedButtonIndex: 0

        Component {
            id: tabButtonRow
            Row {
                id: theRow
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                }
                width: childrenRect.width
                property int rowNumber // set by buttonView

                Component.onCompleted: {
                    buttonView.buttonRowWidth = theRow.width;
                }

                Repeater {
                    id: repeater
                    model: tabs.__tabsModel.children

                    AbstractButton {
                        id: button
                        clip: false
                        width: text.width + text.anchors.leftMargin + text.anchors.rightMargin
                        property bool selected: (index === tabs.selectedTabIndex) &&
                                                (tabBar.active || buttonView.selectedButtonIndex === button.buttonIndex)

                        anchors.top: parent.top
                        height: parent.height - itemStyle.headerTextBottomMargin

                        property real offset: theRow.rowNumber + 1 - button.x / buttonView.buttonRowWidth;
                        property int buttonIndex: index + theRow.rowNumber*repeater.count
                        Component.onCompleted: buttonView.buttons.push(button)

                        Image {
                            id: indicatorImage
                            source: itemStyle.indicatorImageSource
                            anchors {
                                bottom: parent.bottom
                            }
                            x: button.width
                            visible: button.selected && !tabBar.active
                        }

                        Image {
                            source: itemStyle.indicatorImageSource
                            anchors {
                                bottom: parent.bottom
                            }
                            x: button.width
                            property int lastButtonIndex: tabs.selectedTabIndex-1 < 0 ? repeater.count-1 : tabs.selectedTabIndex - 1
                            visible: tabBar.active && (index === lastButtonIndex)
                        }

                        Label {
                            id: text
                            color: selected ? itemStyle.headerTextColorSelected : itemStyle.headerTextColor
                            opacity: (tabBar.active || selected) ? 1.0 : 0

                            Behavior on opacity {
                                NumberAnimation {
                                    duration: itemStyle.headerTextFadeDuration
                                    easing.type: Easing.InOutQuad
                                }
                            }

                            anchors {
                                left: parent.left
                                leftMargin: units.gu(2)
                                rightMargin: units.gu(2)
                                baseline: parent.bottom
                            }
                            text: modelData.title
                            fontSize: itemStyle.headerFontSize
                            font.weight: itemStyle.headerFontWeight
                        }

                        onClicked: {
                            if (!activatingTimer.running) {
                                tabs.selectedTabIndex = index;
                                tabBar.active = false;
                                button.select();
                            } else {
                                activatingTimer.stop();
                            }
                        }

                        // Select this button
                        function select() {
                            buttonView.selectedButtonIndex = button.buttonIndex;
                            buttonView.updateOffset();
                        }
                    }
                }
            }
        }

        delegate: tabButtonRow
        model: 2 // The second buttonRow shows the buttons that disappear on the left
        property bool needsScrolling: buttonRowWidth > tabBar.width
        interactive: needsScrolling
        width: needsScrolling ? tabBar.width : buttonRowWidth
        clip: !needsScrolling // avoid showing the same button twice

        highlightRangeMode: PathView.NoHighlightRange
        offset: 0
        path: Path {
            startX: -buttonView.buttonRowWidth/2
            PathLine {
                x: buttonView.buttonRowWidth*1.5
            }
        }

        // Select the closest of the two buttons that represent the given tab index
        function selectButton(tabIndex) {
            var b1 = buttons[tabIndex];
            var b2 = buttons[tabIndex + tabs.__tabsModel.children.length];

            // find the button with the nearest offset
            var d1 = MathUtils.absDiffMod(b1.offset, buttonView.offset, 2);
            var d2 = MathUtils.absDiffMod(b2.offset, buttonView.offset, 2);
            if (d1 < d2) {
                b1.select();
            } else {
                b2.select();
            }
        }

        function updateOffset() {
            var newOffset = buttonView.buttons[buttonView.selectedButtonIndex].offset;
            if (offset - newOffset < -1) newOffset = newOffset - 2;
            offset = newOffset;
//            indicatorImage.visible = true;
        }

        Behavior on offset {
            SmoothedAnimation {
                velocity: itemStyle.buttonPositioningVelocity
                easing.type: Easing.InOutQuad
            }
        }

        Component.onCompleted: {
            children[1].rowNumber = 0;
            children[2].rowNumber = 1;
            selectButton(tabs.selectedTabIndex);
        }

        // deactivate the tab bar after inactivity
        onMovementStarted: idleTimer.stop()
        onMovementEnded: idleTimer.restart()
        Timer {
            id: idleTimer
            interval: (itemStyle && itemStyle.deactivateTime) ?  itemStyle.deactivateTime : 1000
            running: tabBar.active
            onTriggered: tabBar.active = false
        }
    }

//    Image {
//        id: indicatorImage
//        source: itemStyle.indicatorImageSource
//        anchors {
//            bottom: parent.bottom
//            bottomMargin: itemStyle.headerTextBottomMargin
//        }

//        visible: !tabBar.active
//        opacity: visible ? 1 : 0

//        Behavior on opacity {
//            SequentialAnimation {
//                PropertyAction { property: "opacity"; value: 0 }
//                PauseAnimation { duration: 1000 }
//                NumberAnimation {
//                    from: 0; to: 1; duration: 600
//                    easing.type: Easing.InOutQuad
//                }
//            }
//        }

//        x: getXPosition()

//        function getXPosition() {
//            // getXPosition() gets called before buttonView.button is filled with buttons
//            var buttons = buttonView.children[1].children; // the first buttonRow
//            var selectedButton = buttonView.buttons[tabs.selectedTabIndex];
//            return selectedButton.width + units.gu(2);
//        }
//    }

    MouseArea {
        // an inactive tabBar can be pressed to make it active
        id: mouseArea
        anchors.fill: parent
        enabled: !tabBar.active
        onPressed: {
            tabBar.active = true;
            mouse.accepted = false;
        }
    }
}
