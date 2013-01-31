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
import QtTest 1.0
import Ubuntu.Components 0.1
import Ubuntu.Components.Popups 0.1

TestCase {
    name: "PopupsPopoverAPI"

    function test_show() {
        popOver.show()
    }

    function test_hide() {
        popOver.hide()
    }

    function test_caller() {
        compare(popOver.caller,null,"caller is not set by default")
    }

    function test_dismissArea() {
        compare(popOver.dismissArea, QuickUtils.rootObject, "Default sensing area is root");
    }

    function test_grabDismissAreaEvents() {
        compare(popOver.grabDismissAreaEvents, true, "Default grabs dismiss area events");
    }

    function test_operativeAreaMargins() {
        compare(popOver.leftMargin, 0.0, "No operative left margin by default");
        compare(popOver.topMargin, 0.0, "No operative top margin by default");
        compare(popOver.rightMargin, 0.0, "No operative right margin by default");
        compare(popOver.bottomMargin, 0.0, "No operative bottom margin by default");
    }

    function test_contentWidth() {
        expectFail("", "Content width is wrong due to rootItem not being initialized.");
        compare(popOver.contentWidth, units.gu(40), "Content width is 40 GU");
    }

    // contentHeight testing is not possible as it follows clientRect height

    Popover {
        id: popOver
        Text {
            text: "Hello Popover!"
        }
    }
}
