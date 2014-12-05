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

try:
    from unittest import mock
except ImportError:
    import mock

import logging
import os
from time import sleep

import fixtures
import testtools
from testtools.matchers import Contains

import ubuntuuitoolkit
from ubuntuuitoolkit import tests


class ListItemTestCase(tests.QMLFileAppTestCase):
    path = os.path.abspath(__file__)
    dir_path = os.path.dirname(path)
    test_qml_file_path = os.path.join(
        dir_path, 'test_listitem.ListItemTestCase.qml')

    def setUp(self):
        super(ListItemTestCase, self).setUp()
        self.test_listitem = self.main_view.select_single(
            'UCListItem', objectName='listitem0')
        self.test_page = self.main_view.select_single(
            objectName='test_page')
        self.assertEqual(self.test_page.title, 'No action triggered')

    def test_trigger_delete(self):
        self.test_listitem.trigger_leading_action('delete_action')
        self.assertEqual(self.test_page.title, 'delete_action action triggered')

    def test_trigger_search(self):
        self.test_listitem.trigger_trailing_action('search_action')
        self.assertEqual(self.test_page.title, 'search_action action triggered')

    def test_trigger_edit(self):
        self.test_listitem.trigger_trailing_action('edit_action')
        self.assertEqual(self.test_page.title, 'edit_action action triggered')

    def test_trigger_email(self):
        self.test_listitem.trigger_trailing_action('email_action')
        self.assertEqual(self.test_page.title, 'email_action action triggered')

    def test_trigger_all_actions(self):
        self.test_listitem.trigger_leading_action('delete_action')
        self.assertEqual(self.test_page.title, 'delete_action action triggered')

        self.test_listitem.trigger_trailing_action('search_action')
        self.assertEqual(self.test_page.title, 'search_action action triggered')

        self.test_listitem.trigger_trailing_action('edit_action')
        self.assertEqual(self.test_page.title, 'edit_action action triggered')

        self.test_listitem.trigger_trailing_action('email_action')
        self.assertEqual(self.test_page.title, 'email_action action triggered')

    def test_trigger_nonexistent_leading_action(self):
        error = self.assertRaises(
           ubuntuuitoolkit.ToolkitException,
           self.test_listitem.trigger_leading_action,
           ('this_action_does_not_exist'))
        self.assertEqual(
            str(error),
           'The requested action not found on leading side')

    def test_trigger_nonexistent_trailing_action(self):
        error = self.assertRaises(
           ubuntuuitoolkit.ToolkitException,
           self.test_listitem.trigger_trailing_action,
           'this_action_does_not_exist')
        self.assertEqual(
            str(error),
           'The requested action not found on trailing side')

    def test_select_items(self):
        # Long press to select a word
        # FIXME: input.Mouse doesn't support long press
        # press_duration doesn't work here
        # self.pointing_device.click(press_duration=2.0)
        self.pointing_device.click_object(self.test_listitem)
        self.pointing_device.press()
        sleep(2)
        self.pointing_device.release()
        self.assertTrue(self.test_listitem.selectable)
        self.test_listitem.toggle_selected()
        self.assertTrue(self.test_listitem.selected)