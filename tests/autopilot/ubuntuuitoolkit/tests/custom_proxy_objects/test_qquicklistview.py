# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
#
# Copyright (C) 2013-2015 Canonical Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

from unittest import mock

import os

import testscenarios
from autopilot import platform
from autopilot.introspection import dbus

import ubuntuuitoolkit
from ubuntuuitoolkit import tests


class QQuickListViewTestCase(tests.QMLStringAppTestCase):

    test_qml = ("""
import QtQuick 2.0
import Ubuntu.Components 1.0
import Ubuntu.Components.ListItems 1.0 as ListItem

MainView {
    width: units.gu(48)
    height: units.gu(20)
    objectName: "mainView"

    Page {

        Column {
            id: column
            width: units.gu(48)
            height: units.gu(20)

            Label {
                id: clickedLabel
                objectName: "clickedLabel"
                text: "No element clicked."
            }

            ListView {
                id: testListView
                objectName: "testListView"
                anchors.left: parent.left
                anchors.right: parent.right
                height: column.height - clickedLabel.paintedHeight
                clip: true
                model: 20

                delegate: ListItem.Standard {
                    objectName: "testListElement%1".arg(index)
                    text: "test list element %1".arg(index)
                    onClicked: clickedLabel.text = objectName
                    height: units.gu(5)
                }
            }
        }
    }
}
""")

    def setUp(self):
        super().setUp()
        self.list_view = self.main_view.select_single(
            ubuntuuitoolkit.QQuickListView, objectName='testListView')
        self.label = self.main_view.select_single(
            'Label', objectName='clickedLabel')
        self.assertEqual(self.label.text, 'No element clicked.')

    def test_qquicklistview_custom_proxy_object(self):
        self.assertIsInstance(self.list_view, ubuntuuitoolkit.QQuickListView)

    def test_click_element(self):
        self.list_view.click_element('testListElement0')
        self.assertEqual(self.label.text, 'testListElement0')

    def test_click_element_outside_view_below(self):
        # Click the first element out of view to make sure we are not scrolling
        # to the bottom at once.
        self.assertFalse(
            self.list_view._is_element_clickable('testListElement5'))

        self.list_view.click_element('testListElement5')
        self.assertEqual(self.label.text, 'testListElement5')

    def test_click_element_outside_view_above(self):
        self.list_view.click_element('testListElement9')

        # Click the first element out of view to make sure we are not scrolling
        # to the top at once.
        self.assertFalse(
            self.list_view._is_element_clickable('testListElement4'))

        self.list_view.click_element('testListElement4')
        self.assertEqual(self.label.text, 'testListElement4')

    def test_click_element_not_created_at_start_below(self):
        objectName = 'testListElement19'
        self.assertRaises(
            dbus.StateNotFoundError,
            self.list_view.select_single,
            objectName=objectName)
        self.list_view.click_element(objectName)
        self.assertEqual(self.label.text, 'testListElement19')

    def test_click_element_from_first_page_deleted_when_swiping_down(self):
        """Test that no more swiping is done after finding the element.

        This is a regression test for bug http://pad.lv/1342521 that caused
        us to swipe down after finding an element on the first page of the
        list. If we were lucky, the element was still cashed so we just ended
        up doing two extra swipes. If not, then the element will be deleted
        from the tree and the helper failed.

        """
        # Swipe to the bottom.
        self.list_view.click_element('testListElement19')

        objectName = 'testListElement1'
        self.assertRaises(
            dbus.StateNotFoundError,
            self.list_view.select_single,
            objectName=objectName)
        with mock.patch.object(
                self.list_view,
                'swipe_to_show_more_below') as mock_swipe_down:
            self.list_view.click_element(objectName)
        self.assertFalse(mock_swipe_down.called)
        self.assertEqual(self.label.text, 'testListElement1')

    def test_click_unexisting_element(self):
        error = self.assertRaises(
            ubuntuuitoolkit.ToolkitException,
            self.list_view.click_element,
            'unexisting')
        self.assertEqual(
            str(error), 'List element with objectName "unexisting" not found.')


class QQuickListViewOutOfViewTestCase(tests.QMLStringAppTestCase):
    """Test that we can make elements visible when the list is out of view."""

    test_qml = ("""
import QtQuick 2.0
import Ubuntu.Components 1.0
import Ubuntu.Components.ListItems 1.0 as ListItem

MainView {
    width: units.gu(48)
    height: units.gu(20)
    objectName: "mainView"

    Page {

        Flickable {

            Column {
                id: column
                width: units.gu(48)
                // The column height is greater than the main view height, so
                // the bottom of the list is out of view.
                height: units.gu(40)

                Label {
                    id: clickedLabel
                    objectName: "clickedLabel"
                    text: "No element clicked."
                }

                ListView {
                    id: testListView
                    objectName: "testListView"
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: column.height - clickedLabel.paintedHeight
                    clip: true
                    model: 20

                    delegate: ListItem.Standard {
                        objectName: "testListElement%1".arg(index)
                        text: "test list element %1".arg(index)
                        onClicked: clickedLabel.text = objectName
                        height: units.gu(5)
                    }
                }
            }
        }
    }
}
""")

    def setUp(self):
        super().setUp()
        self.list_view = self.main_view.select_single(
            ubuntuuitoolkit.QQuickListView, objectName='testListView')
        self.label = self.main_view.select_single(
            'Label', objectName='clickedLabel')
        self.assertEqual(self.label.text, 'No element clicked.')

    def test_click_element_outside_view_below(self):
        """Test that we can click an element that's out of view below.

        The list is also out of view, so we must scroll from the bottom of the
        main view.

        """
        # Test for http://pad.lv/1275060.
        # Click the first element out of view to make sure we are not scrolling
        # to the bottom at once.
        self.assertFalse(
            self.list_view._is_element_clickable('testListElement9'))

        self.list_view.click_element('testListElement9')
        self.assertEqual(self.label.text, 'testListElement9')


class QQuickListViewDraggingBaseTestCase(tests.QMLFileAppTestCase):

    def setUp(self):
        path = os.path.abspath(__file__)
        dir_path = os.path.dirname(path)
        self.test_qml_file_path = os.path.join(
            dir_path, self.qml_file_name)

        super().setUp()
        self.list_view = self.main_view.select_single(
            ubuntuuitoolkit.QQuickListView, objectName='test_view')


class QQuickListViewDraggingTestCase(QQuickListViewDraggingBaseTestCase):

    qml_file_name = 'test_listitem.ListViewWithLiveDraggingTestCase.qml'

    def test_long_press_must_enable_drag_mode(self):
        list_item = self.list_view.select_single(
            'UCListItem', objectName='listitem0')
        self.assertFalse(list_item.dragMode)
        self.list_view._enable_drag_mode()
        self.assertTrue(list_item.dragMode)


class QQuickListViewReorderingTestCase(QQuickListViewDraggingBaseTestCase):

    dragging_scenarios = [
        ('with live dragging', {
            'qml_file_name': (
                'test_listitem.ListViewWithLiveDraggingTestCase.qml')}),
        ('without live dragging', {
            'qml_file_name': (
                'test_listitem.ListViewWithoutLiveDraggingTestCase.qml')}),
    ]
    reorder_scenarios = [
        ('both items visible, to bottom', {'from_index': 0, 'to_index': 1}),
        ('both items visible, to top', {'from_index': 1, 'to_index': 0}),
        ('both items visible, to bottom, first non visible', {
            'from_index': 0, 'to_index': 7}),
        ('to item not visible, to top', {'from_index': 15, 'to_index': 0}),
        ('to item not visible, to middle down', {
            'from_index': 0, 'to_index': 15}),
        ('to item not visible, to bottom', {'from_index': 0, 'to_index': 24})
    ]
    scenarios = testscenarios.multiply_scenarios(
        dragging_scenarios, reorder_scenarios)

    def _find_item(self, index):
        object_name = 'listitem{}'.format(index)
        try:
            return self.list_view.select_single(objectName=object_name)
        except dbus.StateNotFoundError:
            return self.list_view._find_element(object_name)

    def _get_item_text(self, index):
        item = self._find_item(index)
        return item.select_single('Label').text

    def test_drag_item_must_reorder_list(self):
        if platform.model() != 'Desktop':
            self.skipTest(
                'Drag does not work on the phone because of bug #1266601')

        original_from_text = self._get_item_text(self.from_index)

        self.list_view.drag_item(self.from_index, self.to_index)

        self.assertEqual(
            original_from_text, self._get_item_text(self.to_index))
