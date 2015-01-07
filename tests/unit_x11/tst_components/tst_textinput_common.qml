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
import Ubuntu.Components 1.1

Item {
    id: testMain
    width: units.gu(40)
    height: units.gu(71)

    Column {
        spacing: units.gu(1)
        TextField {
            id: textField
        }
        TextArea {
            id: textArea
        }
    }

    SignalSpy {
        id: cursorPositionSpy
        signalName: "onCursorPositionChanged"
    }
    SignalSpy {
        id: selectionStartSpy
        signalName: "onSelectionStartChanged"
    }
    SignalSpy {
        id: selectionEndSpy
        signalName: "onSelectionEndChanged"
    }
    SignalSpy {
        id: selectedTextSpy
        signalName: "onSelectedTextChanged"
    }
    SignalSpy {
        id: popupSpy
        signalName: "pressAndHold"
    }
    SignalSpy {
        id: movementXSpy
        signalName: "onContentXChanged"
    }
    SignalSpy {
        id: movementYSpy
        signalName: "contentYChanged"
    }
    SignalSpy {
        id: scrollerSpy
        signalName: "movementEnded"
    }

    UbuntuTestCase {
        name: "TextInputCommonTest"
        when: windowShown

        function init() {
            textField.text = "This is a single line text input called TextField.";
            textArea.text = "This is a multiline text input component called TextArea. It supports fix size as well as auto-expanding behavior. The content is scrollable only if it exceeds the visible area.";
            textField.cursorPosition = 0;
            textArea.cursorPosition = 0;
            waitForRendering(textField, 500);
            waitForRendering(textArea, 500);
        }

        function cleanup() {
            textField.focus = false;
            textArea.focus = false;
            cursorPositionSpy.clear();
            selectionStartSpy.clear();
            selectionEndSpy.clear();
            selectedTextSpy.clear();
            popupSpy.clear();
            movementXSpy.clear();
            movementYSpy.clear();
            cursorRectSpy.clear();
            scrollerSpy.clear();
        }

        function test_clear_text_using_popover_data() {
            return [
                {input: textField},
                {input: textArea},
            ];
        }

        function test_clear_text_using_popover(data) {
            var handler = findChild(data.input, "input_handler");
            popupSpy.target = handler;
            data.input.focus = true;

            // invoke popover
            var x = data.input.width / 2;
            var y = data.input.height / 2;
            mouseClick(data.input, x, y, Qt.RightButton);
            popupSpy.wait();
            var popover = findChild(testMain, "text_input_contextmenu");
            verify(popover, "Cannot retrieve default TextInputPopover");
            waitForRendering(popover);
            // select all
            var selectAll = findChildWithProperty(popover, "text", "Select All");
            verify(selectAll, "Select All item not found");
            mouseClick(selectAll, selectAll.width / 2, selectAll.height / 2);
            waitForRendering(data.input, 1000);
            compare(data.input.text, data.input.selectedText, "Not all the text is selected");
            // delete with key press
            keyClick(Qt.Key_Backspace);
            waitForRendering(data.input, 1000);
            compare(data.input.text, "", "The text has not been deleted");

            // dismiss popover
            mouseClick(testMain, testMain.width / 2, testMain.height / 2);
            wait(200);
        }

        SignalSpy {
            id: cursorRectSpy
            signalName: "cursorRectangleChanged"
        }

        function test_input_pageup_pagedown_data() {
            return [
                {tag: "PageUp in TextField", input: textField, moveToEnd: true, key: Qt.Key_PageUp, modifier: Qt.NoModifier, xfail: false},
                {tag: "PageDown in TextField", input: textField, moveToEnd: false, key: Qt.Key_PageDown, modifier: Qt.NoModifier, xfail: false},
                {tag: "PageUp in TextArea", input: textArea, moveToEnd: true, key: Qt.Key_PageUp, modifier: Qt.NoModifier, xfail: false},
                {tag: "PageDown in TextArea", input: textArea, moveToEnd: false, key: Qt.Key_PageDown, modifier: Qt.NoModifier, xfail: false},
                {tag: "Ctrl+PageUp in TextField", input: textField, moveToEnd: true, key: Qt.Key_PageUp, modifier: Qt.ControlModifier, xfail: true},
                {tag: "Ctrl+PageDown in TextField", input: textField, moveToEnd: false, key: Qt.Key_PageDown, modifier: Qt.ControlModifier, xfail: true},
                {tag: "Ctrl+PageUp in TextArea", input: textArea, moveToEnd: true, key: Qt.Key_PageUp, modifier: Qt.ControlModifier, xfail: true},
                {tag: "Ctrl+PageDown in TextArea", input: textArea, moveToEnd: false, key: Qt.Key_PageDown, modifier: Qt.ControlModifier, xfail: true},
                {tag: "Shift+PageUp in TextField", input: textField, moveToEnd: true, key: Qt.Key_PageUp, modifier: Qt.ShiftModifier, xfail: true},
                {tag: "Shift+PageDown in TextField", input: textField, moveToEnd: false, key: Qt.Key_PageDown, modifier: Qt.ShiftModifier, xfail: true},
                {tag: "Shift+PageUp in TextArea", input: textArea, moveToEnd: true, key: Qt.Key_PageUp, modifier: Qt.ShiftModifier, xfail: true},
                {tag: "Shift+PageDown in TextArea", input: textArea, moveToEnd: false, key: Qt.Key_PageDown, modifier: Qt.ShiftModifier, xfail: true},
                {tag: "Alt+PageUp in TextField", input: textField, moveToEnd: true, key: Qt.Key_PageUp, modifier: Qt.AltModifier, xfail: true},
                {tag: "Alt+PageDown in TextField", input: textField, moveToEnd: false, key: Qt.Key_PageDown, modifier: Qt.AltModifier, xfail: true},
                {tag: "Alt+PageUp in TextArea", input: textArea, moveToEnd: true, key: Qt.Key_PageUp, modifier: Qt.AltModifier, xfail: true},
                {tag: "Alt+PageDown in TextArea", input: textArea, moveToEnd: false, key: Qt.Key_PageDown, modifier: Qt.AltModifier, xfail: true},
            ];
        }
        function test_input_pageup_pagedown(data) {
            var handler = findChild(data.input, "input_handler");
            data.input.focus = true;

            // move the cursor to the end
            if (data.moveToEnd) {
                keyClick(Qt.Key_End);
                waitForRendering(data.input, 500);
                verify(data.input.cursorPosition > 0, "The cursor wasn't moved");
            }
            cursorRectSpy.target = data.input;
            keyClick(data.key, data.modifier);
            waitForRendering(data.input, 500);
            if (data.xfail) {
                expectFailContinue(data.tag, "With modifier");
            }
            cursorRectSpy.wait(500);
            cursorRectSpy.target = null;
        }

        function test_scroll_when_not_focused_data() {
            return [
                // dx and dy are in eights of a degree; see QWheelEvent::angleDelta() for more details.
                {tag: "TextField", input: textField, x: textField.width / 2, y: textField.height / 2, dx: -240, dy: 0},
                {tag: "TextArea", input: textArea, x: textField.width / 2, y: textField.height / 2, dx: 0, dy: -240},
            ];
        }
        function test_scroll_when_not_focused(data) {
            var scroller = findChild(data.input, "input_scroller");
            scrollerSpy.target = scroller;

            mouseWheel(data.input, data.x, data.y, data.dx, data.dy);
            expectFailContinue(data.tag, "Content must not scroll while inactive");
            scrollerSpy.wait(500);
        }

        function test_scroll_when_focused_data() {
            return [
                // dx and dy are in eights of a degree; see QWheelEvent::angleDelta() for more details.
                {tag: "TextField", input: textField, x: textField.width / 2, y: textField.height / 2, dx: -480, dy: 0},
                {tag: "TextArea", input: textArea, x: textArea.width / 2, y: textArea.height / 2, dx: 0, dy: -480},
            ];
        }
        function test_scroll_when_focused(data) {
            var scroller = findChild(data.input, "input_scroller");
            scrollerSpy.target = scroller;

            // focus component
            data.input.focus = true;

            mouseWheel(data.input, data.x, data.y, data.dx, data.dy);
            waitForRendering(data.input);
            scrollerSpy.wait();
        }

        function test_rightclick_opens_popover_data() {
            return [
                {tag: "TextField active", input: textField, whenFocused: true},
                {tag: "TextArea active" , input: textArea, whenFocused: true},
                {tag: "TextField inactive", input: textField, whenFocused: false},
                {tag: "TextArea inactive" , input: textArea, whenFocused: false},
            ];
        }
        function test_rightclick_opens_popover(data) {
            var handler = findChild(data.input, "input_handler");
            popupSpy.target = handler;

            if (data.whenFocused) {
                data.input.focus = true;
                waitForRendering(data.input);
            }
            mouseClick(data.input, data.input.width / 2, data.input.height / 2, Qt.RightButton);
            waitForRendering(data.input);
            popupSpy.wait();
            verify(data.input.cursorPosition !== 0, "Cursor should be moved to the mouse click position.")

            // dismiss popover
            mouseClick(testMain, 0, 0);
            // add some timeout to get the event buffer cleaned
            wait(500);
        }

        function test_clear_selection_on_click_data() {
            return [
                {tag: "TextField click on selection", input: textField, selectChars: 10, clickPos: Qt.point(10, textField.height / 2)},
                {tag: "TextArea click on selection", input: textArea, selectChars: 40, clickPos: Qt.point(20, 20)},
                {tag: "TextField click beside selection", input: textField, selectChars: 5, clickPos: Qt.point(textField.width / 2, textField.height / 2)},
                {tag: "TextArea click beside selection", input: textArea, selectChars: 1, clickPos: Qt.point(textArea.width / 2, textArea.height / 2)},
            ];
        }
        function test_clear_selection_on_click(data) {
            data.input.focus = true;
            data.input.select(0, data.selectChars);
            verify(data.input.selectedText !== "", "No text selected!");

            mouseClick(data.input, data.clickPos.x, data.clickPos.y);
            verify(data.input.selectedText === "", "There is still text selected!");
        }

        function test_select_text_by_mouse_drag_data() {
            return [
                {tag: "TextField", input: textField},
                {tag: "TextArea", input: textArea},
            ];
        }
        function test_select_text_by_mouse_drag(data) {
            data.input.focus = true;

            flick(data.input, 0, 0, data.input.width / 2, data.input.height / 2);
            waitForRendering(data.input);
            verify(data.input.selectedText !== "", "There's no text selected!");
        }

        function test_no_caret_when_no_touchscreen_data() {
            return [
                {tag: "TextField", input: textField},
                {tag: "TextArea", input: textArea},
            ];
        }
        function test_no_caret_when_no_touchscreen(data) {
            if (TestExtras.touchDevicePresent()) {
                skip("This test cannot be executed in touch environment");
            }

            data.input.focus = true;
            waitForRendering(data.input);

            var cursor = findChild(data.input, "textCursor");
            verify(cursor, "Cursor not accessible, FAIL");
            verify(cursor.caret, "No caret is set");
            compare(cursor.caret.visible, false, "Caret must not be visible!");
        }

        function test_select_text_with_double_click_data() {
            return [
                {tag: "TextField", input: textField},
                {tag: "TextArea", input: textArea},
            ];
        }
        function test_select_text_with_double_click(data) {
            data.input.focus = true;
            waitForRendering(data.input, 500);

            mouseDoubleClick(data.input, units.gu(1), units.gu(1));
            waitForRendering(data.input, 500);
            expectFail(data.tag, "mouseDoubleClick() fails to trigger")
            verify(data.input.selectedText != "", "No text selected.");
        }
    }
}
