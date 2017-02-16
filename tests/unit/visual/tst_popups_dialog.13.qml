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
import QtTest 1.0
import Ubuntu.Test 1.3
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3

MainView {
    id: main

    width: units.gu(40)
    height: units.gu(71)

    Button {
        id: pressMe
        anchors.top: parent.top
        text: "Open Dialog"
        onClicked: PopupUtils.open(dialog)
    }

    UbuntuTestCase {
        id: hiddenTest
        name: "Popups.Dialog.Hidden"

        property bool dialogDestroyed: false

        function test_dismiss_dialog_no_focus_window_bug1664620() {
            verify(!hiddenTest.windowShown);
            var dlg = PopupUtils.open(dialog);
            waitForRendering(dlg);
            dlg.Component.destruction.connect(function() { hiddenTest.dialogDestroyed = true });
            keyClick(Qt.Key_Escape);
            tryCompare(hiddenTest, "dialogDestroyed", true, 500, "Dialog not destroyed");
        }
    }

    UbuntuTestCase {
        id: test
        name: "Popups.Dialog"
        when: windowShown

        property bool dialogDestroyed: false

        function test_dismiss_dialog_on_esc_bug1523833() {
            var dlg = PopupUtils.open(dialog);
            waitForRendering(dlg);
            dlg.Component.destruction.connect(function() { test.dialogDestroyed = true });
            keyClick(Qt.Key_Escape);
            tryCompare(test, "dialogDestroyed", true, 500, "Dialog not destroyed");
        }

        function test_focus_restore_ondismiss_dialog() {
            pressMe.forceActiveFocus();

            tryCompare(window, "activeFocusItem", pressMe);

            var dlg = PopupUtils.open(dialog);
            waitForRendering(dlg);

            tryCompare(window, "activeFocusItem", dlg.button);

            keyClick(Qt.Key_Escape);

            tryCompare(window, "activeFocusItem", pressMe);
        }
    }

    Component {
        id: dialog
        Dialog {
            id: ahojDialog
            title: "Ahoj"
            property alias button: closeButton

            Button {
                id: closeButton
                text: "Close"
                onClicked: PopupUtils.close(ahojDialog)
                focus: true
            }
        }
    }
}


