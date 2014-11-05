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

import QtQuick 2.0
import QtTest 1.0
import Ubuntu.Test 1.0
import Ubuntu.Components 1.2

Item {
    id: main
    width: units.gu(50)
    height: units.gu(100)

    Action {
        id: stockAction
        iconName: "starred"
        property var param
        onTriggered: param = value
    }
    ListItemActions {
        id: leading
        actions: [
            Action {
                iconName: "delete"
                property var param
                onTriggered: param = value
            },
            Action {
                iconName: "edit"
                property var param
                onTriggered: param = value
            },
            Action {
                iconName: "camcorder"
                property var param
                onTriggered: param = value
            }
        ]
    }
    ListItemActions {
        id: trailing
        backgroundColor: leading.backgroundColor
        foregroundColor: leading.foregroundColor
        actions: [
            stockAction,
        ]
        delegate: Rectangle {
            objectName: "custom_delegate"
            width: units.gu(10)
            color: "green"
        }
    }
    ListItemActions {
        id: actionsDefault
    }

    Column {
        width: parent.width
        ListItem {
            id: defaults
            width: parent.width
        }
        ListItem {
            id: highlightTest
        }
        ListItem {
            id: testItem
            width: parent.width
            color: "blue"
            leadingActions: leading
            trailingActions: ListItemActions {
                actions: leading.actions
            }
            Item {
                id: bodyItem
                anchors.fill: parent
            }
        }
        ListItem {
            id: controlItem
            Button {
                id: button
                anchors.centerIn: parent
                text: "Button"
            }
        }

        ListView {
            id: listView
            width: parent.width
            height: units.gu(28)
            clip: true
            model: 10
            delegate: ListItem {
                objectName: "listItem" + index
                width: parent.width
                leadingActions: leading
                trailingActions: trailing
            }
        }
        Flickable {
            id: testFlickable
            width: parent.width
            height: units.gu(28)
            ListView {
                id: nestedListView
                width: parent.width
                height: units.gu(28)
                clip: true
                model: 10
                delegate: ListItem {
                    objectName: "listItem" + index
                    leadingActions: leading
                }
            }
        }
    }

    UbuntuTestCase {
        name: "ListItemAPI"
        when: windowShown

        SignalSpy {
            id: movingSpy
            signalName: "movingEnded"
        }

        SignalSpy {
            id: pressedSpy
            signalName: "pressedChanged"
            target: testItem
        }

        SignalSpy {
            id: clickSpy
            signalName: "clicked"
            target: testItem;
        }

        SignalSpy {
            id: actionSpy
            signalName: "triggered"
        }
        SignalSpy {
            id: interactiveSpy
            signalName: "interactiveChanged"
            target: listView
        }

        SignalSpy {
            id: xChangeSpy
            signalName: "xChanged"
        }

        SignalSpy {
            id: draggingSpy
            signalName: "draggingChanged"
        }

        function waitReboundCompletion(item) {
            var prevX;
            tryCompareFunction(function() { var b = prevX == item.contentItem.x; prevX = item.contentItem.x; return b; }, true, 1000);
        }

        function panelItem(actionList) {
            return findInvisibleChild(actionList, "ListItemPanel")
        }

        function rebound(item) {
            movingSpy.target = item;
            movingSpy.clear();
            mouseClick(item, centerOf(item).x, centerOf(item).y);
            if (item.moving) {
                movingSpy.wait();
            }
            movingSpy.target = null;
        }

        function initTestCase() {
            TestExtras.registerTouchDevice();
            waitForRendering(main);
        }

        function cleanup() {
            testItem.action = null;
            testItem.highlightPolicy = ListItem.Automatic;
            testItem.selected = false;
            testItem.selectable = false;
            waitForRendering(testItem, 200);
            movingSpy.clear();
            pressedSpy.clear();
            clickSpy.clear();
            actionSpy.clear();
            xChangeSpy.clear();
            interactiveSpy.target = null;
            interactiveSpy.clear();
            draggingSpy.clear();
            pressAndHoldSpy.clear();
            buttonSpy.clear();
            interactiveSpy.clear();
            listView.interactive = true;
            // make sure we collapse
            mouseClick(defaults, 0, 0)
            movingSpy.target = null;
        }

        function test_0_defaults() {
            verify(defaults.contentItem !== null, "Defaults is null");
            compare(defaults.color, "#000000", "Transparent by default");
            compare(defaults.highlightColor, Theme.palette.selected.background, "Theme.palette.selected.background color by default")
            compare(defaults.pressed, false, "Not pressed buy default");
            compare(defaults.divider.visible, true, "divider is visible by default");
            compare(defaults.divider.leftMargin, units.dp(2), "divider's left margin is 2GU");
            compare(defaults.divider.rightMargin, units.dp(2), "divider's right margin is 2GU");
            compare(defaults.divider.colorFrom, "#000000", "colorFrom differs.");
            fuzzyCompare(defaults.divider.colorFrom.a, 0.14, 0.01, "colorFrom alpha differs");
            compare(defaults.divider.colorTo, "#ffffff", "colorTo differs.");
            fuzzyCompare(defaults.divider.colorTo.a, 0.07, 0.01, "colorTo alpha differs");

            compare(actionsDefault.delegate, null, "ListItemActions has no delegate set by default.");
            compare(actionsDefault.actions.length, 0, "ListItemActions has no actions set.");
            compare(actionsDefault.backgroundColor, Qt.rgba(0, 0, 0, 0), "default background color is transparent");
            compare(actionsDefault.foregroundColor, "#000000", "default foregroundColor must be black");

            compare(actionsDefault.ListItemActions.container, actionsDefault, "The attached container points to the actions list");
            compare(actionsDefault.ListItemActions.listItem, null, "No attached ListItem by default");
            compare(actionsDefault.ListItemActions.listItemIndex, -1, "No attached ListItem index by default");
            compare(actionsDefault.ListItemActions.offset, 0, "No attached offset set by default");
            compare(actionsDefault.ListItemActions.status, ListItemActions.Disconnected, "The attached status is disconnected");
            compare(actionsDefault.ListItemActions.dragging, false, "The attached dragging is false");
        }

        function test_children_in_content_item() {
            compare(bodyItem.parent, testItem.contentItem, "Content is not in the right holder!");
        }

        function test_pressedChanged_on_click() {
            mousePress(testItem, testItem.width / 2, testItem.height / 2);
            pressedSpy.wait();
            mouseRelease(testItem, testItem.width / 2, testItem.height / 2);
        }
        function test_pressedChanged_on_tap() {
            TestExtras.touchPress(0, testItem, centerOf(testItem));
            pressedSpy.wait();
            TestExtras.touchRelease(0, testItem, centerOf(testItem));
            // local cleanup, wait few msecs to suppress double tap
            wait(400);
        }

        function test_clicked_on_mouse() {
            clickSpy.target = testItem;
            mouseClick(testItem, testItem.width / 2, testItem.height / 2);
            clickSpy.wait();
        }
        function test_clicked_on_tap() {
            clickSpy.target = testItem;
            TestExtras.touchClick(0, testItem, centerOf(testItem));
            clickSpy.wait();
        }

        function test_mouse_click_on_listitem() {
            var listItem = findChild(listView, "listItem0");
            verify(listItem, "Cannot find listItem0");

            mousePress(listItem, listItem.width / 2, 0);
            compare(listItem.pressed, true, "Item is not pressed?");
            // do 5 moves to be able to sense it
            var dy = 0;
            for (var i = 1; i <= 5; i++) {
                dy += i * 10;
                mouseMove(listItem, listItem.width / 2, dy);
            }
            compare(listItem.pressed, false, "Item is pressed still!");
            mouseRelease(listItem, listItem.width / 2, dy);
        }
        function test_touch_click_on_listitem() {
            var listItem = findChild(listView, "listItem0");
            verify(listItem, "Cannot find listItem0");

            TestExtras.touchPress(0, listItem, Qt.point(listItem.width / 2, 5));
            compare(listItem.pressed, true, "Item is not pressed?");
            // do 5 moves to be able to sense it
            var dy = 0;
            for (var i = 1; i <= 5; i++) {
                dy += i * 10;
                TestExtras.touchMove(0, listItem, Qt.point(listItem.width / 2, dy));
            }
            compare(listItem.pressed, false, "Item is pressed still!");
            // cleanup, wait few milliseconds to avoid dbl-click collision
            TestExtras.touchRelease(0, listItem, Qt.point(listItem.width / 2, dy));
        }

        function test_background_height_change_on_divider_visible() {
            // make sure the testItem's divider is shown
            testItem.divider.visible = true;
            verify(testItem.contentItem.height < testItem.height, "ListItem's background height must be less than the item itself.");
            testItem.divider.visible = false;
            compare(testItem.contentItem.height, testItem.height, "ListItem's background height must be the same as the item itself.");
            testItem.divider.visible = true;
        }

        function test_touch_tug_actions_data() {
            var item = findChild(listView, "listItem0");
            return [
                {tag: "Trailing, mouse", item: item, pos: centerOf(item), dx: -units.gu(20), positiveDirection: false, mouse: true},
                {tag: "Leading, mouse", item: item, pos: centerOf(item), dx: units.gu(20), positiveDirection: true, mouse: true},
                {tag: "Trailing, touch", item: item, pos: centerOf(item), dx: -units.gu(20), positiveDirection: false, mouse: false},
                {tag: "Leading, touch", item: item, pos: centerOf(item), dx: units.gu(20), positiveDirection: true, mouse: false},
            ];
        }
        function test_touch_tug_actions(data) {
            listView.positionViewAtBeginning();
            movingSpy.target = data.item;
            if (data.mouse) {
                flick(data.item, data.pos.x, data.pos.y, data.dx, 0);
            } else {
                TestExtras.touchDrag(0, data.item, data.pos, Qt.point(data.dx, 0));
            }
            movingSpy.wait();
            if (data.positiveDirection) {
                verify(data.item.contentItem.x > 0, data.tag + " actions did not show up");
            } else {
                verify(data.item.contentItem.x < 0, data.tag + " actions did not show up");
            }

            // dismiss
            rebound(data.item);
        }

        // make sure this is executed as one of the last tests due to requirement to have the panelItem created
        function test_attached_dragging_data() {
            var item = findChild(listView, "listItem0");
            return [
                {tag: "Trailing", item: item, pos: centerOf(item), dx: -units.gu(20), actionList: item.trailingActions},
                {tag: "Leading", item: item, pos: centerOf(item), dx: units.gu(20), actionList: item.leadingActions},
            ];
        }
        function test_attached_dragging(data) {
            listView.positionViewAtBeginning();
            draggingSpy.target = data.actionList.ListItemActions;
            movingSpy.target = data.item;
            flick(data.item, data.pos.x, data.pos.y, data.dx, 0);
            movingSpy.wait();
            compare(draggingSpy.count, 2, "The dragging hadn't been changed twice.");

            // dismiss
            rebound(data.item);
        }

        function test_attached_listitem_data() {
            var item = findChild(listView, "listItem3");
            return [
                {tag: "Trailing", item: item, pos: centerOf(item), dx: -units.gu(20), actionList: item.trailingActions, index: 3},
                {tag: "Leading", item: item, pos: centerOf(item), dx: units.gu(20), actionList: item.leadingActions, index: 3},
            ];
        }
        function test_attached_listitem(data) {
            listView.positionViewAtBeginning();
            movingSpy.target = data.item;
            flick(data.item, data.pos.x, data.pos.y, data.dx, 0);
            movingSpy.wait();
            compare(data.actionList.ListItemActions.listItem, data.item, "The attached listItem differs from the actual item using the list.");
            compare(data.actionList.ListItemActions.listItemIndex, data.index, "The attached listItem index is wrong.");
            verify(data.actionList.ListItemActions.status != ListItemActions.Disconnected, "The attached status is wrong.");

            // dismiss
            rebound(data.item);
        }

        function test_rebound_when_pressed_outside_or_clicked_data() {
            var item0 = findChild(listView, "listItem0");
            var item1 = findChild(listView, "listItem1");
            return [
                {tag: "Click on an other Item", item: item0, pos: centerOf(item0), dx: -units.gu(20), clickOn: item1, mouse: true},
                {tag: "Click on the same Item", item: item0, pos: centerOf(item0), dx: -units.gu(20), clickOn: item0.contentItem, mouse: true},
                {tag: "Tap on an other Item", item: item0, pos: centerOf(item0), dx: -units.gu(20), clickOn: item1, mouse: false},
                {tag: "Tap on the same Item", item: item0, pos: centerOf(item0), dx: -units.gu(20), clickOn: item0.contentItem, mouse: false},
            ];
        }
        function test_rebound_when_pressed_outside_or_clicked(data) {
            listView.positionViewAtBeginning();
            if (data.mouse) {
                flick(data.item, data.pos.x, data.pos.y, data.dx, 0);
            } else {
                TestExtras.touchDrag(0, data.item, data.pos, Qt.point(data.dx, 0));
            }
            waitForRendering(data.item, 400);
            verify(data.item.contentItem.x != 0, "The component wasn't tugged!");
            // dismiss
            rebound(data.item);
        }

        function test_listview_not_interactive_while_tugged_data() {
            var item0 = findChild(listView, "listItem0");
            var item1 = findChild(listView, "listItem1");
            return [
                {tag: "Trailing", item: item0, pos: centerOf(item0), dx: -units.gu(19), dy: units.gu(2), clickOn: item1, mouse: true},
                {tag: "Leading", item: item0, pos: centerOf(item0), dx: units.gu(19), dy: units.gu(2), clickOn: item0.contentItem, mouse: true},
                {tag: "Trailing", item: item0, pos: centerOf(item0), dx: -units.gu(19), dy: units.gu(2), clickOn: item1, mouse: false},
                {tag: "Leading", item: item0, pos: centerOf(item0), dx: units.gu(19), dy: units.gu(2), clickOn: item0.contentItem, mouse: false},
            ];
        }
        function test_listview_not_interactive_while_tugged(data) {
            listView.positionViewAtBeginning();
            interactiveSpy.target = listView;
            compare(listView.interactive, true, "ListView is not interactive");
            movingSpy.target = data.item;
            if (data.mouse) {
                flick(data.item, data.pos.x, data.pos.y, data.dx, data.dy);
            } else {
                TestExtras.touchDrag(0, data.item, data.pos, Qt.point(data.dx, data.dy));
            }
            movingSpy.wait();
            compare(listView.interactive, true, "The ListView is still interactive!");
            // interactive should be changed at least once!
            verify(interactiveSpy.count > 0, "Listview interactive did not change.");
            // dismiss
            rebound(data.item);
        }

        function test_selecting_action_rebounds_data() {
            var item0 = findChild(listView, "listItem0");
            return [
                {tag: "With mouse", item: item0, pos: centerOf(item0), dx: units.gu(20), actions: item0.leadingActions, select: "list_option_0", mouse: true},
                {tag: "With touch", item: item0, pos: centerOf(item0), dx: units.gu(20), actions: item0.leadingActions, select: "list_option_0", mouse: false},
            ]
        }
        function test_selecting_action_rebounds(data) {
            listView.positionViewAtBeginning();
            movingSpy.target = data.item;
            movingSpy.clear();
            if (data.mouse) {
                flick(data.item, data.pos.x, data.pos.y, data.dx, 0);
            } else {
                TestExtras.touchDrag(0, data.item, data.pos, Qt.point(data.dx, 0));
            }
            movingSpy.wait();
            var selectedOption = findChild(panelItem(data.actions), data.select);
            verify(selectedOption, "Cannot select option " + data.select);

            // dismiss
            movingSpy.clear();
            if (data.mouse) {
                mouseClick(selectedOption, centerOf(selectedOption).x, centerOf(selectedOption).y);
            } else {
                TestExtras.touchClick(0, selectedOption, centerOf(selectedOption));
            }
            movingSpy.wait();
            fuzzyCompare(data.item.contentItem.x, 0.0, 0.1, "Did not rebound!");
        }

        function test_custom_trailing_delegate() {
            listView.positionViewAtBeginning();
            var item = findChild(listView, "listItem0");
            movingSpy.target = item;
            flick(item, centerOf(item).x, centerOf(item).y, -units.gu(20), 0);
            verify(panelItem(trailing), "Panel is not visible");
            var custom = findChild(panelItem(trailing), "custom_delegate");
            verify(custom, "Custom delegate not in use");
            movingSpy.wait();
            // cleanup
            rebound(item);
        }

        // execute as last so we make sure we have the panel created
        function test_snap_data() {
            var listItem = findChild(listView, "listItem0");
            verify(listItem, "ListItem cannot be found");
            verify(panelItem(listItem.leadingActions), "Leading panel had not been created!");
            verify(panelItem(listItem.trailingActions), "Trailing panel had not been created!");

            return [
                // the list snaps out if the panel is dragged in > overshoot GU (hardcoded for now)
                {tag: "Snap out leading, mouse", item: listItem, dx: units.gu(2), list: listItem.leadingActions, snap: false},
                {tag: "Snap in leading, mouse", item: listItem, dx: units.gu(4), list: listItem.leadingActions, snap: true},
                {tag: "Snap out trailing, mouse", item: listItem, dx: -units.gu(2), list: listItem.trailingActions, snap: false},
                {tag: "Snap in trailing, mouse", item: listItem, dx: -units.gu(4), list: listItem.trailingActions, snap: true},
            ];
        }
        function test_snap(data) {
            movingSpy.target = data.item;
            flick(data.item, centerOf(data.item).x, centerOf(data.item).y, data.dx, 0);
            movingSpy.wait();
            movingSpy.clear();
            if (data.snap) {
                verify(data.item.contentItem.x != 0.0, "Not snapped to be visible");
                // cleanup
                rebound(data.item);
            } else {
                tryCompareFunction(function() {return data.item.contentItem.x; }, 0.0, 1000, "Not snapped back");
            }
        }

        function test_verify_action_value_data() {
            return [
                // testItem is the child item @index 2 in the topmost Column.
                {tag: "Standalone", item: testItem, result: 2},
                {tag: "Index 0", item: findChild(listView, "listItem0"), result: 0},
                {tag: "Index 1", item: findChild(listView, "listItem1"), result: 1},
                {tag: "Index 2", item: findChild(listView, "listItem2"), result: 2},
                {tag: "Index 3", item: findChild(listView, "listItem3"), result: 3},
            ];
        }
        function test_verify_action_value(data) {
            var option = findChild(panelItem(data.item.leadingActions), "list_option_0");
            verify(option, "actions panel cannot be reached");
            // we test the first action
            var action = data.item.leadingActions.actions[0];
            actionSpy.target = action;
            actionSpy.clear();
            // tug actions in
            movingSpy.target = data.item;
            flick(data.item.contentItem, centerOf(data.item.contentItem).x, centerOf(data.item.contentItem).y, units.gu(5), 0);
            movingSpy.wait();

            // select the option
            movingSpy.clear();
            mouseClick(data.item, centerOf(option).x, centerOf(option).y);
            movingSpy.wait();

            // check the action param
            actionSpy.wait();
            compare(action.param, data.result, "Action parameter differs");
        }

        SignalSpy {
            id: panelItemSpy
            signalName: "onXChanged"
        }

        function test_disabled_item_locked_data() {
            var item0 = findChild(listView, "listItem0");
            return [
                // drag same amount as height is
                {tag: "Simple item, leading", item: testItem, enabled: false, dx: testItem.height},
                {tag: "Simple item, trailing", item: testItem, enabled: false, dx: -testItem.height},
                {tag: "ListView item, leading", item: item0, enabled: false, dx: item0.height},
                {tag: "ListView item, trailing", item: item0, enabled: false, dx: -item0.height},
            ];
        }
        function test_disabled_item_locked(data) {
            var oldEnabled = data.item.enabled;
            panelItemSpy.clear();
            panelItemSpy.target = data.item;
            data.item.enabled = false;
            // tug
            flick(data.item.contentItem, centerOf(data.item.contentItem).x, centerOf(data.item.contentItem).y, data.dx, 0);
            compare(panelItemSpy.count, 0, "Item had been tugged despite being disabled!");
            // check opacity
            fuzzyCompare(data.item.opacity, 0.5, 0.1, "Disabled item must be 50% transparent");
            //cleanup
            data.item.enabled = oldEnabled;
        }

        function test_toggle_selectable_data() {
            return [
                {tag: "When not selected", selected: false},
                {tag: "When selected", selected: true},
            ]
        }
        function test_toggle_selectable(data) {
            xChangeSpy.target = testItem.contentItem;
            testItem.selectable = true;
            waitForRendering(testItem.contentItem, 800);
            testItem.selected = data.selected;
            xChangeSpy.wait();
        }

        function test_no_tug_when_selectable() {
            xChangeSpy.target = null;
            testItem.selectable = true;
            // wait till animation to selection mode ends
            waitReboundCompletion(testItem);

            // try to tug leading
            xChangeSpy.target = testItem.contentItem;
            xChangeSpy.clear();
            compare(xChangeSpy.count, 0, "Wrong signal count!");
            flick(testItem.contentItem, centerOf(testItem.contentItem).x, centerOf(testItem.contentItem).y, testItem.contentItem.width / 2, 0);
            compare(xChangeSpy.count, 0, "No tug allowed when in selection mode");
        }

        SignalSpy {
            id: pressAndHoldSpy
            signalName: "pressAndHold"
        }
        SignalSpy {
            id: buttonSpy
            signalName: "clicked"
            target: button
        }
        function test_pressandhold_suppress_click() {
            var center = centerOf(testItem);
            pressAndHoldSpy.target = testItem;
            clickSpy.target = testItem;
            clickSpy.clear();
            mouseLongPress(testItem, center.x, center.y);
            mouseRelease(testItem, center.x, center.y);
            pressAndHoldSpy.wait();
            compare(clickSpy.count, 0, "Click must be suppressed when long pressed");
        }

        function test_click_on_button_suppresses_listitem_click() {
            buttonSpy.target = button;
            clickSpy.target = controlItem;
            mouseClick(button, centerOf(button).x, centerOf(button).y);
            buttonSpy.wait();
            compare(clickSpy.count, 0, "ListItem clicked() must be suppressed");
        }

        function test_ListItemActions_status_data() {
            var drag = testItem.contentItem.width / 2;
            return [
                {tag:"Leading", item: testItem, dx: drag, list: testItem.leadingActions, expectedStatus: ListItemActions.Leading},
                {tag:"Trailing", item: testItem, dx: -drag, list: testItem.trailingActions, expectedStatus: ListItemActions.Trailing},
            ];
        }
        function test_ListItemActions_status(data) {
            var testItem = data.item.contentItem;
            flick(testItem, centerOf(testItem).x, centerOf(testItem).y, data.dx, 0);
            waitForRendering(testItem, 800);
            compare(data.list.ListItemActions.status, data.expectedStatus, "Status on the option list differs.");
            compare(data.list.ListItemActions.listItem, data.item, "connectedItem is not the tugged item.");
        }

        function test_listitem_blockks_ascendant_flickables() {
            var listItem = findChild(nestedListView, "listItem0");
            verify(listItem, "Cannot find test item");
            interactiveSpy.target = testFlickable;
            movingSpy.target = listItem;
            // tug leading
            flick(listItem, centerOf(listItem).x, centerOf(listItem).y, listItem.width / 2, 0);
            movingSpy.wait();
            // check if interactive got changed
            interactiveSpy.wait();

            // cleanup!!!
            rebound(listItem);
        }

        function test_action_type_set() {
            stockAction.parameterType = Action.None;
            compare(stockAction.parameterType, Action.None, "No parameter type for stockAction!");
            testItem.action = stockAction;
            compare(stockAction.parameterType, Action.Integer, "No parameter type for stockAction!");
        }

        function test_action_triggered_on_clicked() {
            testItem.action = stockAction;
            actionSpy.target = stockAction;
            clickSpy.target = testItem;
            mouseClick(testItem, centerOf(testItem).x, centerOf(testItem).y);
            clickSpy.wait();
            actionSpy.wait();
        }

        function test_action_suppressed_on_longpress() {
            testItem.action = stockAction;
            actionSpy.target = stockAction;
            clickSpy.target = testItem;
            pressAndHoldSpy.target = testItem;
            mouseLongPress(testItem, centerOf(testItem).x, centerOf(testItem).y);
            mouseRelease(testItem, centerOf(testItem).x, centerOf(testItem).y);
            pressAndHoldSpy.wait();
            compare(clickSpy.count, 0, "Click must be suppressed.");
            compare(actionSpy.count, 0, "Action triggered must be suppressed");
        }

        // keep these as last ones so we make sure the panel has been created by the previous swipes
        function test_x_backgroundColor_change() {
            // change panel color for the leading and observe the trailing panelItem color change
            leading.backgroundColor = UbuntuColors.blue;
            compare(findChild(panelItem(leading), "panel_background").color, UbuntuColors.blue, "leading panelItem color differs");
            compare(findChild(panelItem(trailing), "panel_background").color, UbuntuColors.blue, "trailing panelItem color has not been set");
        }
        function test_x_foregroundColor_change() {
            // change panel color for the leading and observe the trailing panelItem color change
            leading.foregroundColor = UbuntuColors.green;
            compare(findChild(panelItem(leading), "action_icon").color, UbuntuColors.green, "leading panelItem color differs");
        }

        // highlight policy
        SignalSpy {
            id: policySpy
        }

        function test_highlight_policy_data() {
            return [
                {tag: "Automatic, empty, click", item: highlightTest, policy: ListItem.Automatic, signal: "clicked", emitted: false},
                {tag: "Automatic, empty, pressAndHold", item: highlightTest, policy: ListItem.Automatic, signal: "pressAndHold", emitted: false},
                {tag: "Automatic, action, click", item: highlightTest, policy: ListItem.Automatic, signal: "clicked", property: "action", value: stockAction, emitted: true},
                {tag: "Automatic, action, pressAndHold", item: highlightTest, policy: ListItem.Automatic, signal: "pressAndHold", property: "action", value: stockAction, emitted: true},

                {tag: "PermanentEnabled, empty, click", item: highlightTest, policy: ListItem.PermanentEnabled, signal: "clicked", emitted: true},
                {tag: "PermanentEnabled, empty, pressAndHold", item: highlightTest, policy: ListItem.PermanentEnabled, signal: "pressAndHold", emitted: true},
                {tag: "PermanentDisabled, action, click", item: highlightTest, policy: ListItem.PermanentDisabled, signal: "clicked", property: "action", value: stockAction, emitted: false},
                {tag: "PermanentDisabled, action, pressAndHold", item: highlightTest, policy: ListItem.PermanentDisabled, signal: "pressAndHold", property: "action", value: stockAction, emitted: false},
                {tag: "PermanentDisabled, leadingActions, click", item: highlightTest, policy: ListItem.PermanentDisabled, signal: "clicked", property: "leadingActions", value: leading, emitted: false},
                {tag: "PermanentDisabled, leadingActions, pressAndHold", item: highlightTest, policy: ListItem.PermanentDisabled, signal: "pressAndHold", property: "trailingActions", value: trailing, emitted: false},
            ];
        }
        function test_highlight_policy(data) {
            var prevPolicy = data.item.highlightPolicy;
            data.item.highlightPolicy = data.policy;
            if (data.property) {
                data.item[data.property] = data.value;
            }
            policySpy.signalName = data.signal;
            policySpy.target = data.item;
            policySpy.clear();

            // perform action
            if (data.signal === "clicked") {
                mouseClick(data.item, centerOf(data.item).x, centerOf(data.item).y);
            } else if (data.signal === "pressAndHold") {
                mouseLongPress(data.item, centerOf(data.item).x, centerOf(data.item).y);
                mouseRelease(data.item, centerOf(data.item).x, centerOf(data.item).y);
            }
            if (data.emitted) {
                policySpy.wait();
            } else {
                compare(policySpy.count, 0, "Signal is emitted!");
            }
            // cleanup
            data.item.highlightPolicy = prevPolicy;
        }
    }
}
