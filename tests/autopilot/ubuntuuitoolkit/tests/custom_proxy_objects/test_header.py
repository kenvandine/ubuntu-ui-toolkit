# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
#
# Copyright (C) 2013, 2014 Canonical Ltd.
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

import logging
import os

import fixtures
from testtools.matchers import Contains

import ubuntuuitoolkit
from ubuntuuitoolkit import tests


class HeaderTestCase(tests.QMLFileAppTestCase):

    path = os.path.abspath(__file__)
    dir_path = os.path.dirname(path)
    tools_test_qml_file_path = os.path.join(
        dir_path, 'test_header.HeaderToolsTestCase.qml')
    actions_test_qml_file_path = os.path.join(
        dir_path, 'test_header.HeaderActionsTestCase.qml')

    scenarios = [
        ('deprecated tools',
            dict(test_qml_file_path=tools_test_qml_file_path)),
        ('actions',
            dict(test_qml_file_path=actions_test_qml_file_path))
    ]

    def setUp(self):
        super(HeaderTestCase, self).setUp()
        self.header = self.main_view.get_header()
        self.label = self.app.select_single(
            'Label', objectName='clicked_label')
        self.assertEqual(self.label.text, 'No button clicked.')

    def test_header_custom_proxy_object(self):
        self.assertIsInstance(self.header, ubuntuuitoolkit.AppHeader)
        self.assertTrue(self.header.visible)
        self.assertEqual(self.header.title, "Test title")

    def test_click_header_action_button(self):
        self.header.click_action_button('action0')
        self.assertEqual(self.label.text, 'Button 0 clicked.')

    def test_click_header_overflow_action_button(self):
        # custom back button and first action button go in the header
        # and the others in the overflow.
        self.header.click_action_button('action3')
        self.assertEqual(self.label.text, 'Button 3 clicked.')

    def test_click_unexisting_header_action_button(self):
        error = self.assertRaises(
            ubuntuuitoolkit.ToolkitException, self.header.click_action_button,
            'unexisting')
        self.assertEqual(
            str(error),
            'Button not found in header or overflow')

    # back button and tabs button are tested in pagestack and tabs tests
    def test_click_custom_back_button(self):
        self.header.click_custom_back_button()
        self.assertEqual(self.label.text, 'Cancel button clicked.')

    def test_click_header_action_button_with_hidden_header(self):
        bottom_label = self.main_view.select_single(objectName='end_label')
        bottom_label.swipe_into_view()
        self.assertFalse(self.header._is_visible())
        self.header.click_action_button('action0')
        self.assertEqual(self.label.text, 'Button 0 clicked.')

    def test_overflow_button(self):
        # there are 5 actions plus a custom back action
        overflow_button = self.header.select_single(
            'AbstractButton',
            objectName='actions_overflow_button')
        self.assertEqual(overflow_button.visible, True)

        hide_actions_button = self.main_view.select_single(
            'Button',
            objectName='hide_actions_button')
        self.pointing_device.click_object(hide_actions_button)

        # only three actions are visible
        self.assertEqual(overflow_button.visible, False)


class CustomMainView(ubuntuuitoolkit.MainView):
    """Autopilot helper for a custom main view."""


class HeaderInCustomMainViewTestCase(tests.QMLFileAppTestCase):

    path = os.path.abspath(__file__)
    dir_path = os.path.dirname(path)
    test_qml_file_path = os.path.join(
        dir_path, 'test_header.HeaderInCustomMainViewTestCase.qml')

    @property
    def main_view(self):
        return self.app.select_single(CustomMainView)

    def test_get_header_from_custom_main_view(self):
        """Test that we can get the header from a custom main view.

        This prevents a regression of http://pad.lv/1324556.

        """
        header = self.main_view.get_header()
        self.assertIsInstance(header, ubuntuuitoolkit.AppHeader)


class DeprecatedHeaderTestCase(tests.QMLFileAppTestCase):

    path = os.path.abspath(__file__)
    dir_path = os.path.dirname(path)
    test_qml_file_path = os.path.join(
        dir_path, 'test_header.DeprecatedHeaderTestCase.qml')

    @property
    def main_view(self):
        return self.app.select_single('QQuickItem', objectName='main')

    def test_get_deprecated_header_must_log_deprecation_warning(self):
        fake_logger = fixtures.FakeLogger(level=logging.WARNING)
        self.useFixture(fake_logger)
        self.main_view.select_single(ubuntuuitoolkit.Header)
        self.assertThat(
            fake_logger.output,
            Contains(
                'Header is an internal QML component of Ubuntu.Components and '
                'its API may change or be removed at any moment. Please use '
                'MainView and Page instead.'))
