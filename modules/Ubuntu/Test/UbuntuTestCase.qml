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

/*!
    \qmlabstract UbuntuTestCase
    \inqmlmodule Ubuntu.Test 0.1
    \ingroup ubuntu
    \brief The UbuntuTestCase class expands the default TestCase class.

    \b{This component is under heavy development.}

    This class extends the default QML TestCase class which is available in QtTest 1.0.
*/
TestCase {
	TestUtil {
		id:util
	}

	/*!
		Find a child from the item based on the objectName.
	*/
	function findChild(obj,objectName) {
		var childs = new Array(0);
		childs.push(obj)
		while (childs.length > 0) {
			if (childs[0].objectName == objectName) {
				return childs[0]
			}
			for (var i in childs[0].children) {
				childs.push(childs[0].children[i])
			}
			childs.splice(0, 1);
		}
		return null;
	}

	function findInvisibleChild(obj,objectName) {
		var childs = new Array(0);
		childs.push(obj)
		while (childs.length > 0) {
			if (childs[0].objectName == objectName) {
				return childs[0]
			}
			for (var i in childs[0].data) {
				childs.push(childs[0].data[i])
			}
			childs.splice(0, 1);
		}
		return null;
	}

	/*!
		Move Mouse from x,y to distance of dx, dy divided to steps with a stepdelay (ms).
	*/
	function mouseMoveSlowly(item,x,y,dx,dy,steps,stepdelay) {
		mouseMove(item,x,y);
		var step_dx = dx/steps;
		var step_dy = dy/steps;

		var ix = 0;
		var iy = 0;
		for (var step=0; step<steps; step++) {
			if (ix<dx) {
				ix += step_dx;
			}
			if (iy<dx) {
				iy += step_dy;
			}
			mouseMove(item,x + ix,y + iy,stepdelay);
		}
	}

	/*!
		Keeps executing a given parameter-less function until it returns the given
		expected result or the timemout is reached (in which case a test failure
		is generated)
	*/

	function tryCompareFunction(func, expectedResult, timeout) {
		var timeSpent = 0
		var success = false
		var actualResult
		if (timeout == undefined) {
                    timeout = 5000;
                }
		while (timeSpent < timeout && !success) {
			actualResult = func()
			success = qtest_compareInternal(actualResult, expectedResult)
			if (success === false) {
				wait(50)
				timeSpent += 50
			}
		}

		var act = qtest_results.stringify(actualResult)
		var exp = qtest_results.stringify(expectedResult)
		if (!qtest_results.compare(success,
				"function returned unexpected result",
				act, exp,
				util.callerFile(), util.callerLine())) {
			throw new Error("QtQuickTest::fail")
		}
	}
} 

