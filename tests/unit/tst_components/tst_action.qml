/*
 * Copyright 2013 Canonical Ltd.
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
import Ubuntu.Components 1.1
// FIXME: do cleanup https://bugs.launchpad.net/ubuntu-ui-toolkit/+bug/1369874
import Ubuntu.Unity.Action 1.1 as Unity

TestCase {
     name: "ActionAPI"

     function contains(list, entry) {
         for (var i = 0; i < list.length; i++) {
             if (list[i] == entry) {
                 return true;
             }
         }
         return false;
     }

     function initTestCase() {
         compare(action.text, "", "text is empty string set by default")
         compare(action.iconSource, "", "iconSource is empty string by default")
         compare(action.iconName, "", "iconName is empty string by default")
     }

     function test_iconSource() {
         compare(action.iconSource, "", "iconSource is empty string by default")
         var newIconSource = "../../../examples/ubuntu-ui-toolkit-gallery/small_avatar.png"
         action.iconSource = newIconSource
         compare(action.iconSource, Qt.resolvedUrl(newIconSource), "iconSource can be set")
         action.iconSource = ""
         compare(action.iconSource, "", "iconSource can be unset")
     }

     function test_iconName() {
         compare(action.iconName, "", "iconName is empty string by default")
         var newIconName = "compose"
         action.iconName = newIconName
         compare(action.iconName, newIconName, "iconName can be set")
         action.iconName = ""
         compare(action.iconName, "", "iconName can be unset")
     }

     function test_trigger() {
         compare(triggeredSignalSpy.count, 0)
         action.triggered(null);
         compare(triggeredSignalSpy.count, 1)
     }

     function test_signal_triggered_exists() {
         compare(triggeredSignalSpy.valid, true, "triggered signal exists")
     }

     function test_valid_value_type_data() {
         return [
            {tag: "None", type: Action.None, param: undefined},
            {tag: "String", type: Action.String, param: "test"},
            {tag: "Integer", type: Action.Integer, param: 100},
            {tag: "Bool", type: Action.Bool, param: true},
            {tag: "Real", type: Action.Real, param: 12.34},
            {tag: "Object - QtObject", type: Action.Object, param: object},
            {tag: "Object - Item", type: Action.Object, param: item},
         ];
     }
     function test_valid_value_type(data) {
         valueType.parameterType = data.type;
         valueType.trigger(data.param);
         valueTypeSpy.wait();
         compare(valueType.parameter, data.param, "Test " + data.tag + " result differs");
         valueTypeSpy.clear();
     }

     function test_invalid_value_type_data() {
         return [
            {tag: "None", type: Action.None, param: 120},
            {tag: "String", type: Action.String, param: object},
            {tag: "Integer", type: Action.Integer, param: "100"},
            {tag: "Bool", type: Action.Bool, param: item},
            {tag: "Real", type: Action.Real, param: undefined},
            {tag: "Object - QtObject", type: Action.Object, param: true},
            {tag: "Object - Item", type: Action.Object, param: "item"},
         ];
     }
     function test_invalid_value_type(data) {
         valueType.parameterType = data.type;
         valueType.trigger(data.param);
         valueTypeSpy.wait();
         compare(valueType.parameter, undefined, "Test " + data.tag + " did not fail");
         valueTypeSpy.clear();
     }

     function test_actionmanager() {
         verify(manager.globalContext, "Global context is not defined");
         compare(manager.localContexts.length, 2, "Invalid number of local contexts defined");
     }

     function test_globalcontext_actions() {
         compare(manager.globalContext.actions.length, 3, "Global context action count must be a sum of all manager's actions' counts");
     }

     function test_add_unity_actioncontext_failure() {
         manager.addLocalContext(unityContext);
         verify(!contains(manager.localContexts, unityContext), "Unity ActionContext cannot be added");
     }

     function test_unity_action_not_in_context() {
         verify(!contains(manager.globalContext.actions, unityAction, "Unity Action cannot be registered"));
     }

     function test_cannot_add_unity_action_to_global_context() {
         manager.globalContext.addAction(stockUnityAction);
         verify(!contains(manager.globalContext.actions, stockUnityAction, "Unity Action cannot be registered"));
     }

     function test_cannot_add_unity_action_to_local_context() {
         context1.addAction(stockUnityAction);
         verify(!contains(context1.actions, stockUnityAction, "Unity Action cannot be registered"));
     }
     function test_activate_contexts_data() {
         return [
             {tag: "Activate context1", active: context1, inactive: context2},
             {tag: "Activate context2", active: context2, inactive: context1},
             {tag: "Activate context1 again", active: context1, inactive: context2},
         ];
     }
     function test_activate_contexts(data) {
         data.active.active = true;
         verify(data.active.active, "Context activation error");
         verify(!data.inactive.active, "Context deactivation error");
     }

     Action {
         id: action
     }
     Action {
         id: valueType
         property var parameter
         onTriggered: parameter = value
     }
     Unity.Action {
         id: stockUnityAction
     }

     QtObject {
         id: object
     }
     Item {
         id: item
     }

     SignalSpy {
         id: triggeredSignalSpy
         target: action
         signalName: "triggered"
     }
     SignalSpy {
         id: valueTypeSpy
         target: valueType
         signalName: "triggered"
     }
     SignalSpy {
         id: textSpy
         target: action
         signalName: "textChanged"
     }

     ActionManager {
         id: manager
     }

     ActionManager {
         id: manager2
         Action {
         }
         Action {
         }
         Action {
         }
         Unity.Action {
             id: unityAction
         }
     }

     ActionContext {
         id: context1
     }
     ActionContext {
         id: context2
     }

     Unity.ActionContext {
         id: unityContext
     }
}