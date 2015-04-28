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
import Ubuntu.Components.Popups 1.0
import Ubuntu.Components.ListItems 1.0 as ListItem
import Ubuntu.Components.Styles 1.2 as Style

Style.PageHeadStyle {
    id: headerStyle
    objectName: "PageHeadStyle" // used in unit tests
    contentHeight: units.gu(6)
    fontWeight: Font.Light
    fontSize: "large"
    textLeftMargin: units.gu(2)
    maximumNumberOfActions: 3

    /*!
      The color of the buttons in the header.
     */
    property color buttonColor: styledItem.config.foregroundColor

    /*!
      The color of the title text.
     */
    property color titleColor: styledItem.config.foregroundColor

    // FIXME: When the three panel color properties below are removed,
    //  update unity8/Dash/PageHeader to use the new theming (currently
    //  in progress) to set these colors.
    /*!
      \deprecated
      The background color of the tabs panel and the actions overflow panel.
     */
    property color panelBackgroundColor: styledItem.panelColor

    /*!
       \deprecated
       The background color of the tapped item in the panel.
      */
    property color panelHighlightColor: theme.palette.selected.background

    /*!
       \deprecated
       The foreground color (icon and text) of actions in the panel.
      */
    property color panelForegroundColor: theme.palette.selected.backgroundText

    /*!
      The text color of unselected sections and the section divider.
     */
    property color sectionColor: theme.palette.selected.backgroundText

    /*!
      The text color of the selected section.
     */
    property color selectedSectionColor: UbuntuColors.orange

    /*!
      The background color of the pressed section.
     */
    property color sectionHighlightColor: theme.palette.selected.background

    implicitHeight: headerStyle.contentHeight + divider.height + sectionsItem.height

    /*!
      The height of the row displaying the sections, if sections are specified.
     */
    property real sectionsHeight: units.gu(4)

    // FIXME: Workaround to get sectionsRepeater.count in autopilot tests,
    //  see also FIXME in AppHeader where this property is used.
    property alias __sections_repeater_for_autopilot: sectionsRepeater

    // Used by unit tests and autopilot tests to wait for animations to finish
    readonly property bool animating: headerStyle.state == "OUT"
                                      || leftAnchor.anchors.leftMargin < 0

    // for Unity8
    // FIXME: Remove this property when we introduce a header preset that does not
    //  have a separator.
    property alias __separator_visible: divider.visible

    Rectangle {
        id: divider
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: units.dp(1)
        color: styledItem.dividerColor
    }

    Item {
        id: sectionsItem
        anchors {
            bottom: divider.top
            left: parent.left
            right: parent.right
        }

        visible: sectionsItem.sections.model !== undefined
        height: visible ? headerStyle.sectionsHeight : 0

        property PageHeadSections sections: styledItem.config.sections

        Row {
            id: sectionsRow
            anchors {
                top: parent.top
                bottom: parent.bottom
                horizontalCenter: parent.horizontalCenter
            }
            width: childrenRect.width
            enabled: sectionsItem.sections.enabled
            visible: sectionsItem.sections.model !== undefined
            opacity: enabled ? 1.0 : 0.5

            Repeater {
                id: sectionsRepeater
                model: sectionsItem.sections.model
                objectName: "page_head_sections_repeater"
                AbstractButton {
                    id: sectionButton
                    anchors {
                        top: parent.top
                        bottom: parent.bottom
                    }
                    objectName: "section_button_" + index
                    enabled: sectionsRow.enabled
                    width: label.width + units.gu(4) // FIXME: expose spacing as style property
                    property bool selected: index === sectionsItem.sections.selectedIndex
                    onClicked: sectionsItem.sections.selectedIndex = index;

                    Rectangle {
                        visible: parent.pressed
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            right: parent.right
                        }
                        height: sectionsRow.height
                        color: headerStyle.sectionHighlightColor
                    }

                    Label {
                        id: label
                        text: modelData
                        fontSize: "small"
                        anchors.centerIn: parent
                        horizontalAlignment: Text.AlignHCenter
                        color: sectionButton.selected ?
                                   headerStyle.selectedSectionColor :
                                   headerStyle.sectionColor
                    }

                    Rectangle {
                        id: sectionLine
                        anchors {
                            bottom: parent.bottom
                            left: parent.left
                            right: parent.right
                        }
                        height: units.dp(2) // FIXME: Expose as style property
                        color: sectionButton.selected ?
                                   headerStyle.selectedSectionColor :
                                   styledItem.dividerColor
                    }
                }
            }
        }
    }

    states: [
        State {
            name: "IN"
            PropertyChanges {
                target: allContents
                opacity: 1.0
            }
        },
        State {
            name: "OUT"
            PropertyChanges {
                target: allContents
                opacity: 0.0
            }
        }
    ]

    function animateOut() {
        state = "OUT";
    }
    function animateIn() {
        state = "IN";
    }

    signal animateOutFinished()
    signal animateInFinished()

    transitions: [
        Transition {
            id: transitionOut
            from: "IN"
            to: "OUT"
            SequentialAnimation {
                ParallelAnimation {
                    UbuntuNumberAnimation {
                        target: allContents
                        property: "opacity"
                        from: 1.0
                        to: 0.0
                    }
                    UbuntuNumberAnimation {
                        target: leftAnchor
                        properties: "anchors.leftMargin"
                        from: 0.0
                        to: -units.gu(5)
                    }
                    UbuntuNumberAnimation {
                        target: rightAnchor
                        properties: "anchors.rightMargin"
                        from: 0
                        to: -units.gu(5)
                    }
                }
                ScriptAction {
                    script: headerStyle.animateOutFinished()
                }
            }
        },
        Transition {
            id: transitionIn
            from: "OUT"
            to: "IN"
            SequentialAnimation {
                ParallelAnimation {
                    UbuntuNumberAnimation {
                        target: allContents
                        property: "opacity"
                        from: 0.0
                        to: 1.0
                    }
                    UbuntuNumberAnimation {
                        target: leftAnchor
                        properties: "anchors.leftMargin"
                        from: -units.gu(5)
                        to: 0
                    }
                    UbuntuNumberAnimation {
                        target: rightAnchor
                        properties: "anchors.rightMargin"
                        from: -units.gu(5)
                        to: 0
                    }
                }
                ScriptAction {
                    script: headerStyle.animateInFinished()
                }
            }
        }
    ]

    Item {
        id: allContents
        anchors.fill: parent

        Item {
            id: leftAnchor
            anchors {
                top: parent.top
                bottom: parent.bottom
                left: parent.left
                leftMargin: 0
            }
            width: 0
        }
        Item {
            id: rightAnchor
            anchors {
                top: parent.top
                bottom: parent.bottom
                right: parent.right
                rightMargin: 0
            }
            width: 0
        }

        Item {
            id: leftButtonContainer
            anchors {
                left: leftAnchor.right
                top: parent.top
                leftMargin: width > 0 ? units.gu(1) : 0
            }
            width: childrenRect.width
            height: headerStyle.contentHeight

            PageHeadButton {
                id: customBackButton
                objectName: "customBackButton"
                action: styledItem.config.backAction
                visible: null !== styledItem.config.backAction &&
                         styledItem.config.backAction.visible
                color: styledItem.config.foregroundColor
            }

            PageHeadButton {
                id: backButton
                objectName: "backButton"

                iconName: "back"
                visible: styledItem.pageStack !== null &&
                         styledItem.pageStack !== undefined &&
                         styledItem.pageStack.depth > 1 &&
                         !styledItem.config.backAction

                text: "back"
                color: styledItem.config.foregroundColor

                onTriggered: {
                    styledItem.pageStack.pop();
                }
            }

            PageHeadButton {
                id: tabsButton
                objectName: "tabsButton"

                iconName: "navigation-menu"
                visible: styledItem.tabsModel !== null &&
                         styledItem.tabsModel !== undefined &&
                         !backButton.visible &&
                         !customBackButton.visible
                text: visible ? styledItem.tabsModel.count + " tabs" : ""
                color: headerStyle.buttonColor

                onTriggered: PopupUtils.open(tabsPopoverComponent, tabsButton)

                Component {
                    id: tabsPopoverComponent

                    OverflowPanel {
                        id: tabsPopover
                        objectName: "tabsPopover"
                        tabsOverflow: true
                        model: styledItem.tabsModel
                        backgroundColor: headerStyle.panelBackgroundColor
                        foregroundColor: headerStyle.panelForegroundColor
                        highlightColor: headerStyle.panelHighlightColor
                    }
                }
            }
        }

        Item {
            id: foreground
            anchors {
                left: leftButtonContainer.right
                top: parent.top
                // don't keep a margin if there is already a button with spacing
                leftMargin: leftButtonContainer.width > 0 ? 0 : headerStyle.textLeftMargin
            }
            width: parent.width - anchors.leftMargin
                   - leftButtonContainer.anchors.leftMargin - leftButtonContainer.width
                   - actionsContainer.anchors.rightMargin - actionsContainer.width
            height: headerStyle.contentHeight

            Label {
                objectName: "header_title_label"
                LayoutMirroring.enabled: Qt.application.layoutDirection == Qt.RightToLeft
                visible: !contentsContainer.visible && styledItem.config.preset === ""
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
                text: styledItem.title
                font.weight: headerStyle.fontWeight
                fontSize: headerStyle.fontSize
                color: headerStyle.titleColor
                elide: Text.ElideRight
            }

            Item {
                // This Item is used to make the custom header item invisible
                // when styledItem.contents is unset and its parent is not updated
                // when the bindings below is no longer active
                id: contentsContainer
                anchors.fill: parent
                visible: styledItem.contents || styledItem.config.contents
            }
            Binding {
                target: styledItem.contents
                property: "anchors.fill"
                value: contentsContainer
                when: styledItem.contents
            }
            Binding {
                target: styledItem.contents
                property: "parent"
                value: contentsContainer
                when: styledItem.contents
            }
            Binding {
                target: styledItem.config.contents
                property: "parent"
                value: contentsContainer
                when: styledItem.config.contents && !styledItem.contents
            }
        }

        Row {
            id: actionsContainer

            property var visibleActions: getVisibleActions(styledItem.config.actions)
            function getVisibleActions(actions) {
                var visibleActionList = [];
                for (var i in actions) {
                    var action = actions[i];
                    if (action && action.hasOwnProperty("visible") && action.visible) {
                        visibleActionList.push(action);
                    }
                }
                return visibleActionList;
            }

            QtObject {
                id: numberOfSlots
                property int requested: actionsContainer.visibleActions.length
                property int left: tabsButton.visible || backButton.visible ||
                                   customBackButton.visible ? 1 : 0
                property int right: headerStyle.maximumNumberOfActions - left
                property int overflow: actionsOverflowButton.visible ? 1 : 0
                property int used: Math.min(right - overflow, requested)
            }

            anchors {
                top: parent.top
                right: rightAnchor.left
                rightMargin: actionsContainer.width > 0 ? units.gu(1) : 0
            }
            width: childrenRect.width
            height: headerStyle.contentHeight

            Repeater {
                model: numberOfSlots.used
                PageHeadButton {
                    id: actionButton
                    objectName: action.objectName + "_header_button"
                    action: actionsContainer.visibleActions[index]
                    color: headerStyle.buttonColor
                    state: styledItem.config.preset === "select" ?
                               "IconAndLabel" : ""
                }
            }

            PageHeadButton {
                id: actionsOverflowButton
                objectName: "actions_overflow_button"
                visible: numberOfSlots.requested > numberOfSlots.right
                // Ensure resetting of X when this button is not visible to avoid
                // miscalculation of actionsContainer.width. Fixes bug #1408481.
                onVisibleChanged: if (!visible) x = 0
                iconName: "contextual-menu"
                color: headerStyle.buttonColor
                height: actionsContainer.height
                onTriggered: PopupUtils.open(actionsOverflowPopoverComponent, actionsOverflowButton)

                Component {
                    id: actionsOverflowPopoverComponent

                    OverflowPanel {
                        id: actionsOverflowPopover
                        objectName: "actions_overflow_popover"

                        backgroundColor: headerStyle.panelBackgroundColor
                        foregroundColor: headerStyle.panelForegroundColor
                        highlightColor: headerStyle.panelHighlightColor

                        // Ensure the popover closes when actions change and
                        // the list item below may be destroyed before its
                        // onClicked is executed. See bug
                        // https://bugs.launchpad.net/ubuntu-ui-toolkit/+bug/1326963
                        Connections {
                            target: styledItem.config
                            onActionsChanged: {
                                actionsOverflowPopover.hide();
                            }
                        }
                        Connections {
                            target: styledItem
                            onConfigChanged: {
                                actionsOverflowPopover.hide();
                            }
                        }

                        tabsOverflow: false
                        model: actionsContainer.visibleActions.slice(numberOfSlots.used,
                                                                     numberOfSlots.requested)
                    }
                }
            }
        }
    }
}
