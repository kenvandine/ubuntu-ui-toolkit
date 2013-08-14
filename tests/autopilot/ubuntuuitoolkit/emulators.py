# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
#
# Copyright (C) 2012, 2013 Canonical Ltd.
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

from autopilot import input, platform
from autopilot.introspection import dbus

_NO_TABS_ERROR = 'The MainView has no Tabs.'


class ToolkitEmulatorException(Exception):
    """Exception raised when there is an error with the emulator."""


def get_pointing_device():
    """Return the pointing device depending on the platform.

    If the platform is `Desktop`, the pointing device will be a `Mouse`.
    If not, the pointing device will be `Touch`.

    """
    if platform.model == 'Desktop':
        input_device_class = input.Mouse
    else:
        input_device_class = input.Touch
    return input.Pointer(input_device_class.create())


class UbuntuUIToolkitEmulatorBase(dbus.CustomEmulatorBase):
    """A base class for all the Ubuntu UI Toolkit emulators."""


class MainView(UbuntuUIToolkitEmulatorBase):
    """MainView Autopilot emulator."""

    def __init__(self, *args):
        super(MainView, self).__init__(*args)
        self.pointing_device = get_pointing_device()

    def get_header(self):
        """Return the Header emulator of the MainView."""
        return self.select_single('Header', objectName='MainView_Header')

    def get_toolbar(self):
        """Return the Toolbar emulator of the MainView."""
        return self.select_single(Toolbar)

    def open_toolbar(self):
        """Open the toolbar if it's not already opened.

        :return: The toolbar.

        """
        toolbar = self.get_toolbar()
        toolbar.animating.wait_for(False)
        if not toolbar.opened:
            self._drag_to_open_toolbar()
            toolbar.opened.wait_for(True)
            toolbar.animating.wait_for(False)

        return toolbar

    def _drag_to_open_toolbar(self):
        x, y, _, _ = self.globalRect
        line_x = x + self.width * 0.50
        start_y = y + self.height - 1
        stop_y = y + self.height * 0.95

        self.pointing_device.drag(line_x, start_y, line_x, stop_y)

    def close_toolbar(self):
        """Close the toolbar if it's opened."""
        toolbar = self.get_toolbar()
        toolbar.animating.wait_for(False)
        if toolbar.opened:
            self._drag_to_close_toolbar()
            toolbar.opened.wait_for(False)
            toolbar.animating.wait_for(False)

    def _drag_to_close_toolbar(self):
        x, y, _, _ = self.globalRect
        line_x = x + self.width * 0.50
        start_y = y + self.height * 0.95
        stop_y = y + self.height - 1

        self.pointing_device.drag(line_x, start_y, line_x, stop_y)

    def get_tabs(self):
        """Return the Tabs emulator of the MainView."""
        tabs = self.select_single(Tabs)
        assert tabs is not None, _NO_TABS_ERROR
        return tabs

    def switch_to_next_tab(self):
        """Open the next tab.

        :return: The newly opened tab.

        """
        self.get_header().switch_to_next_tab()
        current_tab = self.get_tabs().get_current_tab()
        current_tab.visible.wait_for(True)
        return current_tab

    def switch_to_tab_by_index(self, index):
        """Open a tab.

        :parameter index: The index of the tab to open.
        :return: The newly opened tab.

        """
        tabs = self.get_tabs()
        number_of_tabs = tabs.get_number_of_tabs()
        if index >= number_of_tabs:
            raise IndexError('Tab index out of range.')
        current_tab = tabs.get_current_tab()
        number_of_switches = 0
        while not tabs.selectedTabIndex == index:
            if number_of_switches >= number_of_tabs - 1:
                # This prevents a loop. But if this error is ever raised, it's
                # likely there's a bug on the emulator or on the QML Tab.
                raise ToolkitEmulatorException(
                    'The tab with index {0} was not selected.'.format(index))
            current_tab = self.switch_to_next_tab()
            number_of_switches += 1
        return current_tab

    def switch_to_previous_tab(self):
        """Open the previous tab.

        :return: The newly opened tab.

        """
        tabs = self.get_tabs()
        if tabs.selectedTabIndex == 0:
            previous_tab_index = tabs.get_number_of_tabs() - 1
        else:
            previous_tab_index = tabs.selectedTabIndex - 1
        return self.switch_to_tab_by_index(previous_tab_index)

    def switch_to_tab(self, object_name):
        """Open a tab.

        :parameter object_name: The QML objectName property of the tab.
        :return: The newly opened tab.

        """
        tabs = self.get_tabs()
        for index, tab in enumerate(tabs.select_many('Tab')):
            if tab.objectName == object_name:
                return self.switch_to_tab_by_index(index)
        raise ValueError(
            'Tab with objectName "{0}" not found.'.format(object_name))

    def get_action_selection_popover(self, object_name):
        """Return an ActionSelectionPopover emulator.

        :parameter object_name: The QML objectName property of the popover.

        """
        return self.select_single(
            ActionSelectionPopover, objectName=object_name)


class Header(UbuntuUIToolkitEmulatorBase):
    """Header Autopilot emulator."""

    def __init__(self, *args):
        super(Header, self).__init__(*args)
        self.pointing_device = get_pointing_device()

    def _get_animating(self):
        tab_bar_style = self.select_single('TabBarStyle')
        return tab_bar_style.animating

    def switch_to_next_tab(self):
        """Open the next tab."""
        tab_bar = self.select_single(TabBar)
        assert tab_bar is not None, _NO_TABS_ERROR
        tab_bar.switch_to_next_tab()

        # Sleep while the animation finishes.
        self._get_animating().wait_for(False)


class Toolbar(UbuntuUIToolkitEmulatorBase):
    """Toolbar Autopilot emulator."""

    def __init__(self, *args):
        super(Toolbar, self).__init__(*args)
        self.pointing_device = get_pointing_device()

    def click_button(self, object_name):
        """Click a button of the toolbar.

        :param object_name: The QML objectName property of the button.

        """
        button = self._get_button(object_name)
        if button is None:
            raise ValueError(
                'Button with objectName "{0}" not found.'.format(object_name))
        self.pointing_device.click_object(button)

    def _get_button(self, object_name):
        return self.select_single('ActionItem', objectName=object_name)


class Tabs(UbuntuUIToolkitEmulatorBase):
    """Tabs Autopilot emulator."""

    def get_current_tab(self):
        """Return the currently selected tab."""
        return self.select_many('Tab')[self.selectedTabIndex]

    def get_number_of_tabs(self):
        """Return the number of tabs."""
        return len(self.select_many('Tab'))


class TabBar(UbuntuUIToolkitEmulatorBase):
    """TabBar Autopilot emulator."""

    def __init__(self, *args):
        super(TabBar, self).__init__(*args)
        self.pointing_device = get_pointing_device()

    def switch_to_next_tab(self):
        """Open the next tab."""
        # Click the tab bar to switch to selection mode.
        self.pointing_device.click_object(self)
        self.pointing_device.click_object(self._get_next_tab_button())

    def _get_next_tab_button(self):
        current_index = self._get_selected_button_index()
        next_index = (current_index + 1) % self._get_number_of_tab_buttons()
        return self._get_tab_buttons()[next_index]

    def _get_selected_button_index(self):
        return self.select_single('QQuickPathView').selectedButtonIndex

    def _get_number_of_tab_buttons(self):
        return len(self._get_tab_buttons())

    def _get_tab_buttons(self):
        return self.select_many('AbstractButton')


class ActionSelectionPopover(UbuntuUIToolkitEmulatorBase):
    """ActionSelectionPopover Autopilot emulator."""

    def __init__(self, *args):
        super(ActionSelectionPopover, self).__init__(*args)
        self.pointing_device = get_pointing_device()

    def click_button_by_text(self, text):
        """Click a button on the popover.

        XXX We are receiving the text because there's no way to set the
        objectName on the action. This is reported at
        https://bugs.launchpad.net/ubuntu-ui-toolkit/+bug/1205144
        --elopio - 2013-07-25

        :parameter text: The text of the button.

        """
        assert self.visible, 'The popover is not open.'
        button = self._get_button(text)
        if button is None:
            raise ValueError(
                'Button with text "{0}" not found.'.format(text))
        self.pointing_device.click_object(button)
        if self.autoClose:
            self.visible.wait_for(False)

    def _get_button(self, text):
        buttons = self.select_many('Empty')
        for button in buttons:
            if button.text == text:
                return button
