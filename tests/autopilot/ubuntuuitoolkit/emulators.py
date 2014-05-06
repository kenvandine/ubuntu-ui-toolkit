# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
#
# Copyright (C) 2012, 2013, 2014 Canonical Ltd.
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

import os
import logging
from distutils import version

import autopilot
from autopilot import (
    input,
    logging as autopilot_logging,
    platform
)
from autopilot.introspection import dbus

import base
from time import sleep

_NO_TABS_ERROR = 'The MainView has no Tabs.'

logger = logging.getLogger(__name__)


class ToolkitEmulatorException(Exception):
    """Exception raised when there is an error with the emulator."""


def get_pointing_device():
    """Return the pointing device depending on the platform.

    If the platform is `Desktop`, the pointing device will be a `Mouse`.
    If not, the pointing device will be `Touch`.

    """
    if platform.model() == 'Desktop':
        input_device_class = input.Mouse
    else:
        input_device_class = input.Touch
    return input.Pointer(device=input_device_class.create())


def get_keyboard():
    """Return the keyboard device."""
    # TODO return the OSK if we are on the phone. --elopio - 2014-01-13
    return input.Keyboard.create()


def check_autopilot_version():
    """Check that the Autopilot installed version matches the one required.

    :raise ToolkitEmulatorException: If the installed Autopilot version does't
        match the required by the emulators.

    """
    installed_version = version.LooseVersion(autopilot.version)
    if installed_version < version.LooseVersion('1.4'):
        raise ToolkitEmulatorException(
            'The emulators need Autopilot 1.4 or higher.')


# Containers helpers.

def _get_visible_container_top(containers):
    containers_top = [container.globalRect.y for container in containers]
    return max(containers_top)


def _get_visible_container_bottom(containers):
    containers_bottom = [
        container.globalRect.y + container.globalRect.height
        for container in containers if container.globalRect.height > 0]
    return min(containers_bottom)


class UbuntuUIToolkitEmulatorBase(dbus.CustomEmulatorBase):
    """A base class for all the Ubuntu UI Toolkit emulators."""

    def __init__(self, *args):
        check_autopilot_version()
        super(UbuntuUIToolkitEmulatorBase, self).__init__(*args)
        self.pointing_device = get_pointing_device()


class MainView(UbuntuUIToolkitEmulatorBase):
    """MainView Autopilot emulator."""

    def get_header(self):
        """Return the Header emulator of the MainView."""
        try:
            return self.select_single('Header', objectName='MainView_Header')
        except dbus.StateNotFoundError:
            raise ToolkitEmulatorException('The main view has no header.')

    def get_toolbar(self):
        """Return the Toolbar emulator of the MainView."""
        return self.select_single(Toolbar)

    @autopilot_logging.log_action(logger.info)
    def open_toolbar(self):
        """Open the toolbar if it's not already opened.

        :return: The toolbar.

        """
        return self.get_toolbar().open()

    @autopilot_logging.log_action(logger.info)
    def close_toolbar(self):
        """Close the toolbar if it's opened."""
        self.get_toolbar().close()

    def get_tabs(self):
        """Return the Tabs emulator of the MainView.

        :raise ToolkitEmulatorException: If the main view has no tabs.

        """
        try:
            return self.select_single(Tabs)
        except dbus.StateNotFoundError:
            raise ToolkitEmulatorException(_NO_TABS_ERROR)

    @autopilot_logging.log_action(logger.info)
    def switch_to_next_tab(self):
        """Open the next tab.

        :return: The newly opened tab.

        """
        logger.debug('Switch to next tab.')
        self.get_header().switch_to_next_tab()
        current_tab = self.get_tabs().get_current_tab()
        current_tab.visible.wait_for(True)
        return current_tab

    def _switch_to_tab_in_deprecated_tabbar_by_index(self, index):
        tabs = self.get_tabs()
        number_of_tabs = tabs.get_number_of_tabs()
        if index >= number_of_tabs:
            raise ToolkitEmulatorException('Tab index out of range.')

        current_tab = tabs.get_current_tab()
        number_of_switches = 0
        while not tabs.selectedTabIndex == index:
            logger.debug(
                'Current tab index: {0}.'.format(tabs.selectedTabIndex))
            if number_of_switches >= number_of_tabs - 1:
                # This prevents a loop. But if this error is ever raised, it's
                # likely there's a bug on the emulator or on the QML Tab.
                raise ToolkitEmulatorException(
                    'The tab with index {0} was not selected.'.format(index))
            current_tab = self.switch_to_next_tab()
            number_of_switches += 1
        return current_tab

    def _switch_to_tab_in_drawer_by_index(self, index):
        tabs = self.get_tabs()
        number_of_tabs = tabs.get_number_of_tabs()
        if index >= number_of_tabs:
            raise ToolkitEmulatorException('Tab index out of range.')

        if (index != tabs.selectedTabIndex):
            self.get_header().switch_to_tab_by_index(index)
        current_tab = tabs.get_current_tab()
        return current_tab

    @autopilot_logging.log_action(logger.info)
    def switch_to_tab_by_index(self, index):
        """Open a tab.

        :parameter index: The index of the tab to open.
        :return: The newly opened tab.
        :raise ToolkitEmulatorException: If the tab index is out of range.

        """
        logger.debug('Switch to tab with index {0}.'.format(index))

        if (self.useDeprecatedToolbar):
            return self._switch_to_tab_in_deprecated_tabbar_by_index(index)
        else:
            return self._switch_to_tab_in_drawer_by_index(index)

    @autopilot_logging.log_action(logger.info)
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

    @autopilot_logging.log_action(logger.info)
    def switch_to_tab(self, object_name):
        """Open a tab.

        :parameter object_name: The QML objectName property of the tab.
        :return: The newly opened tab.
        :raise ToolkitEmulatorException: If there is no tab with that object
            name.

        """
        tabs = self.get_tabs()
        for index, tab in enumerate(tabs.select_many('Tab')):
            if tab.objectName == object_name:
                return self.switch_to_tab_by_index(tab.index)
        raise ToolkitEmulatorException(
            'Tab with objectName "{0}" not found.'.format(object_name))

    def get_action_selection_popover(self, object_name):
        """Return an ActionSelectionPopover emulator.

        :parameter object_name: The QML objectName property of the popover.

        """
        return self.select_single(
            ActionSelectionPopover, objectName=object_name)

    @autopilot_logging.log_action(logger.info)
    def go_back(self):
        """Go to the previous page."""
        if self.useDeprecatedToolbar:
            toolbar = self.open_toolbar()
            toolbar.click_back_button()
        else:
            self.get_header().click_back_button()


class Header(UbuntuUIToolkitEmulatorBase):
    """Header Autopilot emulator."""

    def __init__(self, *args):
        super(Header, self).__init__(*args)
        self.pointing_device = get_pointing_device()

    def click_back_button(self):
        if self.useDeprecatedToolbar:
            raise ToolkitEmulatorException('Old header has no back button')
        try:
            back_button = self.select_single(
                'AbstractButton', objectName='backButton')
        except dbus.StateNotFoundError:
            raise ToolkitEmulatorException('Missing back button in header')
        if not back_button.visible:
            raise ToolkitEmulatorException('Back button in header not visible')
        self.pointing_device.click_object(back_button)

    def _get_animating(self):
        if (self.useDeprecatedToolbar):
            tab_bar_style = self.select_single('TabBarStyle')
            return tab_bar_style.animating
        else:
            return False

    def _switch_to_next_tab_in_deprecated_tabbar(self):
        try:
            tab_bar = self.select_single(TabBar)
        except dbus.StateNotFoundError:
            raise ToolkitEmulatorException(_NO_TABS_ERROR)
        tab_bar.switch_to_next_tab()
        self._get_animating().wait_for(False)

    def _switch_to_next_tab_in_drawer(self):
        tabs_model_properties = self.select_single(
            'QQuickItem', objectName='tabsModelProperties')
        next_tab_index = (tabs_model_properties.selectedIndex
                          + 1) % tabs_model_properties.count
        self._switch_to_tab_in_drawer_by_index(next_tab_index)

    @autopilot_logging.log_action(logger.info)
    def switch_to_next_tab(self):
        """Open the next tab.

        :raise ToolkitEmulatorException: If the main view has no tabs.

        """
        if (self.useDeprecatedToolbar):
            self._switch_to_next_tab_in_deprecated_tabbar()
        else:
            self._switch_to_next_tab_in_drawer()

    def _switch_to_tab_in_drawer_by_index(self, index):
        try:
            tabs_drawer_button = self.select_single(
                'AbstractButton', objectName='tabsButton')
        except dbus.StateNotFoundError:
            raise ToolkitEmulatorException(_NO_TABS_ERROR)
        self.pointing_device.click_object(tabs_drawer_button)

        tabs_model_properties = self.select_single(
            'QQuickItem', objectName='tabsModelProperties')

        if (tabs_model_properties.selectedIndex == index):
            return

        try:
            tab_button = self.get_root_instance().select_single(
                'Standard', objectName='tabButton' + str(index))
        except dbus.StateNotFoundError:
            raise ToolkitEmulatorException(
                "Tab button {0} not found.".format(index))

        self.pointing_device.click_object(tab_button)

    @autopilot_logging.log_action(logger.info)
    def switch_to_tab_by_index(self, index):
        """Open a tab. This only supports the new tabs in the header

        :parameter index: The index of the tab to open.
        :raise ToolkitEmulatorException: If the tab index is out of range or
                useDeprecatedToolbar is set.

        """
        if (self.useDeprecatedToolbar):
            raise ToolkitEmulatorException(
                "Header.swtich_to_tab_by_index only works with new header")
        else:
            self._switch_to_tab_in_drawer_by_index(index)


class Toolbar(UbuntuUIToolkitEmulatorBase):
    """Toolbar Autopilot emulator."""

    @autopilot_logging.log_action(logger.info)
    def open(self):
        """Open the toolbar if it's not already opened.

        :return: The toolbar.

        """
        self.animating.wait_for(False)
        if not self.opened:
            self._drag_to_open()
            self.opened.wait_for(True)
            self.animating.wait_for(False)

        return self

    def _drag_to_open(self):
        x, y, _, _ = self.globalRect
        line_x = x + self.width * 0.50
        start_y = y + self.height - 1
        stop_y = y

        self.pointing_device.drag(line_x, start_y, line_x, stop_y)

    @autopilot_logging.log_action(logger.info)
    def close(self):
        """Close the toolbar if it's opened."""
        self.animating.wait_for(False)
        if self.opened:
            self._drag_to_close()
            self.opened.wait_for(False)
            self.animating.wait_for(False)

    def _drag_to_close(self):
        x, y, _, _ = self.globalRect
        line_x = x + self.width * 0.50
        start_y = y
        stop_y = y + self.height - 1

        self.pointing_device.drag(line_x, start_y, line_x, stop_y)

    @autopilot_logging.log_action(logger.info)
    def click_button(self, object_name):
        """Click a button of the toolbar.

        The toolbar should be opened before clicking the button, or an
        exception will be raised. If the toolbar is closed for some reason
        (e.g., timer finishes) after moving the mouse cursor and before
        clicking the button, it is re-opened automatically by this function.

        :parameter object_name: The QML objectName property of the button.
        :raise ToolkitEmulatorException: If there is no button with that object
            name.

        """
        # ensure the toolbar is open
        if not self.opened:
            raise ToolkitEmulatorException(
                'Toolbar must be opened before calling click_button().')
        try:
            button = self._get_button(object_name)
        except dbus.StateNotFoundError:
            raise ToolkitEmulatorException(
                'Button with objectName "{0}" not found.'.format(object_name))
        self.pointing_device.move_to_object(button)
        # ensure the toolbar is still open (may have closed due to timeout)
        self.open()
        # click the button
        self.pointing_device.click_object(button)

    def _get_button(self, object_name):
        return self.select_single('ActionItem', objectName=object_name)

    @autopilot_logging.log_action(logger.info)
    def click_back_button(self):
        """Click the back button of the toolbar."""
        self.click_button('back_toolbar_button')


class Tabs(UbuntuUIToolkitEmulatorBase):
    """Tabs Autopilot emulator."""

    def get_current_tab(self):
        """Return the currently selected tab."""
        return self._get_tab(self.selectedTabIndex)

    def _get_tab(self, index):
        tabs = self._get_tabs()
        for tab in tabs:
            if tab.index == index:
                return tab
        else:
            raise ToolkitEmulatorException(
                'There is no tab with index {0}.'.format(index))

    def _get_tabs(self):
        return self.select_many('Tab')

    def get_number_of_tabs(self):
        """Return the number of tabs."""
        return len(self._get_tabs())


class TabBar(UbuntuUIToolkitEmulatorBase):
    """TabBar Autopilot emulator."""

    @autopilot_logging.log_action(logger.info)
    def switch_to_next_tab(self):
        """Open the next tab."""
        self._activate_tab_bar()
        logger.debug('Click the next tab bar button.')
        self.pointing_device.click_object(self._get_next_tab_button())

    def _activate_tab_bar(self):
        # First move to the tab bar to avoid timing issues when we find it in
        # selection mode but it's deselected while we move to it.
        self.pointing_device.move_to_object(self)
        if self.selectionMode:
            logger.debug('Already in selection mode.')
        else:
            # Click the tab bar to switch to selection mode.
            logger.debug('Click the tab bar to enable selection mode.')
            self.pointing_device.click_object(self)

    def _get_next_tab_button(self):
        current_index = self._get_selected_button_index()
        next_index = (current_index + 1) % self._get_number_of_tab_buttons()
        return self._get_tab_button(next_index)

    def _get_selected_button_index(self):
        return self.select_single('QQuickPathView').selectedButtonIndex

    def _get_number_of_tab_buttons(self):
        return len(self._get_tab_buttons())

    def _get_tab_buttons(self):
        return self.select_many('AbstractButton')

    def _get_tab_button(self, index):
        buttons = self._get_tab_buttons()
        for button in buttons:
            if button.buttonIndex == index:
                return button
        raise ToolkitEmulatorException(
            'There is no tab button with index {0}.'.format(index))


class ActionSelectionPopover(UbuntuUIToolkitEmulatorBase):
    """ActionSelectionPopover Autopilot emulator."""

    def click_button_by_text(self, text):
        """Click a button on the popover.

        XXX We are receiving the text because there's no way to set the
        objectName on the action. This is reported at
        https://bugs.launchpad.net/ubuntu-ui-toolkit/+bug/1205144
        --elopio - 2013-07-25

        :parameter text: The text of the button.
        :raise ToolkitEmulatorException: If the popover is not open.

        """
        if not self.visible:
            raise ToolkitEmulatorException('The popover is not open.')
        button = self._get_button(text)
        if button is None:
            raise ToolkitEmulatorException(
                'Button with text "{0}" not found.'.format(text))
        self.pointing_device.click_object(button)
        if self.autoClose:
            try:
                self.visible.wait_for(False)
            except dbus.StateNotFoundError:
                # The popover was removed from the tree.
                pass

    def _get_button(self, text):
        buttons = self.select_many('Empty')
        for button in buttons:
            if button.text == text:
                return button


class CheckBox(UbuntuUIToolkitEmulatorBase):
    """CheckBox Autopilot emulator."""

    @autopilot_logging.log_action(logger.info)
    def check(self, timeout=10):
        """Check a CheckBox, if its not already checked.

        :parameter timeout: number of seconds to wait for the CheckBox to be
            checked. Default is 10.

        """
        if not self.checked:
            self.change_state(timeout)

    @autopilot_logging.log_action(logger.info)
    def uncheck(self, timeout=10):
        """Uncheck a CheckBox, if its not already unchecked.

        :parameter timeout: number of seconds to wait for the CheckBox to be
            unchecked. Default is 10.

        """
        if self.checked:
            self.change_state(timeout)

    @autopilot_logging.log_action(logger.info)
    def change_state(self, timeout=10):
        """Change the state of a CheckBox.

        If it is checked, it will be unchecked. If it is unchecked, it will be
        checked.

        :parameter time_out: number of seconds to wait for the CheckBox state
            to change. Default is 10.

        """
        original_state = self.checked
        self.pointing_device.click_object(self)
        self.checked.wait_for(not original_state, timeout)


class ComboButton(UbuntuUIToolkitEmulatorBase):
    """ComboButton Autopilot emulator."""

    def press_mainbutton(self):
        """Presses the main button of the ComboBox."""
        main_button = self.select_single(objectName="combobutton_mainbutton")
        self.pointing_device.click_object(main_button)

    def expand(self):
        """Expands a combo button by clicking on the dropdown button."""
        if not self.expanded:
            self._press_dropdown()
        return self.expanded

    def collapse(self):
        """Collapses a combo button by clicking on the dropdown button."""
        if self.expanded:
            self._press_dropdown()
        return self.expanded

    def _press_dropdown(self):
        """Presses the dropdown button to togle combo list expansion."""
        dropdown_button = self.select_single(objectName="combobutton_dropdown")
        return self.pointing_device.click_object(dropdown_button)


class OptionSelector(UbuntuUIToolkitEmulatorBase):
    """OptionSelector Autopilot emulator"""

    def get_option_count(self):
        """Gets the number of items in the option selector"""
        self.list_view = self.select_single("QQuickListView")
        return self.list_view.count

    def get_selected_index(self):
        """Gets the current selected index of the QQuickListView"""
        self.list_view = self.select_single("QQuickListView")
        return self.list_view.currentIndex

    def get_selected_text(self):
        """gets the text of the currently selected item"""
        option_selector_delegate = self.select_single(
            'OptionSelectorDelegate', focus='True')
        current_label = option_selector_delegate.select_single(
            'Label', visible='True')
        return current_label.text

    def get_current_label(self):
        """gets the text of the currently selected item"""
        option_selector_delegate = self.select_single(
            'OptionSelectorDelegate', focus='True')
        current_label = option_selector_delegate.select_single(
            'Label', visible='True')
        return current_label

    def _expand(self):
        """Expand an optionselector if it's collapsed"""
        # if just collapsed it can think that the item is expanded
        #  https://bugs.launchpad.net/ubuntu-ui-toolkit/+bug/1240288
        sleep(1)
        if not self.expanded and not self.currentlyExpanded:
            self.pointing_device.click_object(self.get_current_label())
            self.currentlyExpanded.wait_for(True)
            # selecting the same item too quickly after expand
            # causes the wrong item to be selected
            # https://bugs.launchpad.net/ubuntu-ui-toolkit/+bug/1231939
            sleep(1)

    def select_option(self, *args, **kwargs):
        """Select delegate in option selector

        Example usage::
            select_option(objectName="myOptionSelectorDelegate")
            select_option('Label', text="some_text_here")

        :parameter kwargs: keywords used to find property(s) of delegate in
            option selector

        """

        if args:
            try:
                select_object = self.select_single(
                    *args,
                    **kwargs
                )
            except dbus.StateNotFoundError:
                raise ToolkitEmulatorException(
                    'OptionSelectorDelegate with args {} and kwargs {} not '
                    'found'.format(args, kwargs)
                )

        else:
            try:
                select_object = self.select_single(
                    'OptionSelectorDelegate',
                    **kwargs
                )
            except dbus.StateNotFoundError:
                raise ToolkitEmulatorException(
                    'OptionSelectorDelegate with kwargs {} not found'.format(
                        kwargs)
                )

        self._expand()
        self.pointing_device.click_object(select_object)


class TextField(UbuntuUIToolkitEmulatorBase):
    """TextField Autopilot emulator."""

    def __init__(self, *args):
        super(TextField, self).__init__(*args)
        self.keyboard = get_keyboard()

    def write(self, text, clear=True):
        """Write into the text field.

        :parameter text: The text to write.
        :parameter clear: If True, the text field will be cleared before
            writing the text. If False, the text will be appended at the end
            of the text field. Default is True.

        """
        with self.keyboard.focused_type(self):
            self.focus.wait_for(True)
            if clear:
                self.clear()
            else:
                if not self.is_empty():
                    self.keyboard.press_and_release('End')
            self.keyboard.type(text)

    def clear(self):
        """Clear the text field."""
        if not self.is_empty():
            if self.hasClearButton:
                self._click_clear_button()
            else:
                self._clear_with_keys()
            self.text.wait_for('')

    def is_empty(self):
        """Return True if the text field is empty. False otherwise."""
        return self.text == ''

    def _click_clear_button(self):
        clear_button = self.select_single(
            'AbstractButton', objectName='clear_button')
        if not clear_button.visible:
            self.pointing_device.click_object(self)
        self.pointing_device.click_object(clear_button)

    def _clear_with_keys(self):
        if platform.model() == 'Desktop':
            self._select_all()
        else:
            # Touch tap currently doesn't have a press_duration parameter, so
            # we can't show the popover. Reported as bug http://pad.lv/1268782
            # --elopio - 2014-01-13
            self.keyboard.press_and_release('End')
        while not self.is_empty():
            # We delete with backspace because the on-screen keyboard has that
            # key.
            self.keyboard.press_and_release('BackSpace')

    def _select_all(self):
        self.pointing_device.click_object(self, press_duration=1)
        root = self.get_root_instance()
        main_view = root.select_single(MainView)
        popover = main_view.get_action_selection_popover('text_input_popover')
        popover.click_button_by_text('Select All')


class Flickable(UbuntuUIToolkitEmulatorBase):

    @autopilot_logging.log_action(logger.info)
    def swipe_child_into_view(self, child):
        """Make the child visible.

        Currently it works only when the object needs to be swiped vertically.
        TODO implement horizontal swiping. --elopio - 2014-03-21

        """
        containers = self._get_containers()
        if not self._is_child_visible(child, containers):
            self._swipe_non_visible_child_into_view(child, containers)
        else:
            logger.debug('The element is already visible.')

    def _get_containers(self):
        """Return a list with the containers to take into account when swiping.

        The list includes this flickable and the top-most container.
        TODO add additional flickables that are between this and the top
        container. --elopio - 2014-03-22

        """
        containers = [self._get_top_container(), self]
        return containers

    def _get_top_container(self):
        """Return the top-most container with a globalRect."""
        root = self.get_root_instance()
        containers = [root]
        while len(containers) == 1:
            try:
                containers[0].globalRect
                return containers[0]
            except AttributeError:
                containers = containers[0].get_children()

        raise ToolkitEmulatorException("Couldn't find the top-most container.")

    def _is_child_visible(self, child, containers):
        """Check if the center of the child is visible.

        :return: True if the center of the child is visible, False otherwise.

        """
        object_center = child.globalRect.y + child.globalRect.height // 2
        visible_top = _get_visible_container_top(containers)
        visible_bottom = _get_visible_container_bottom(containers)
        return (object_center >= visible_top and
                object_center <= visible_bottom)

    @autopilot_logging.log_action(logger.info)
    def _swipe_non_visible_child_into_view(self, child, containers):
        while not self._is_child_visible(child, containers):
            # Check the direction of the swipe based on the position of the
            # child relative to the immediate flickable container.
            if child.globalRect.y < self.globalRect.y:
                self._swipe_to_show_more_above(containers)
            else:
                self._swipe_to_show_more_below(containers)

    @autopilot_logging.log_action(logger.info)
    def _swipe_to_show_more_above(self, containers):
        if self.atYBeginning:
            raise ToolkitEmulatorException(
                "Can't swipe more, we are already at the top of the "
                "container.")
        else:
            self._swipe_to_show_more('above', containers)

    @autopilot_logging.log_action(logger.info)
    def _swipe_to_show_more_below(self, containers):
        if self.atYEnd:
            raise ToolkitEmulatorException(
                "Can't swipe more, we are already at the bottom of the "
                "container.")
        else:
            self._swipe_to_show_more('below', containers)

    def _swipe_to_show_more(self, direction, containers):
        start_x = stop_x = self.globalRect.x + (self.globalRect.width // 2)
        # Start and stop just a little under the top and a little over the
        # bottom.
        top = _get_visible_container_top(containers) + 5
        bottom = _get_visible_container_bottom(containers) - 5
        if direction == 'below':
            start_y = bottom
            stop_y = top
        elif direction == 'above':
            start_y = top
            stop_y = bottom
        else:
            raise ToolkitEmulatorException(
                'Invalid direction {}.'.format(direction))
        self._slow_drag(start_x, stop_x, start_y, stop_y)
        self.dragging.wait_for(False)
        self.moving.wait_for(False)

    def _slow_drag(self, start_x, stop_x, start_y, stop_y):
        # If we drag too fast, we end up scrolling more than what we
        # should, sometimes missing the  element we are looking for.
        self.pointing_device.drag(start_x, start_y, stop_x, stop_y, rate=5)

    @autopilot_logging.log_action(logger.info)
    def _scroll_to_top(self):
        if not self.atYBeginning:
            containers = self._get_containers()
            while not self.atYBeginning:
                self._swipe_to_show_more_above(containers)


class QQuickListView(Flickable):

    @autopilot_logging.log_action(logger.info)
    def click_element(self, object_name):
        """Click an element from the list.

        It swipes the element into view if it's center is not visible.

        :parameter objectName: The objectName property of the element to click.

        """
        try:
            element = self.select_single(objectName=object_name)
        except dbus.StateNotFoundError:
            # The element might be on a part of the list that hasn't been
            # created yet. We have to search for it scrolling the entire list.
            element = self._find_element(object_name)
        self.swipe_child_into_view(element)
        self.pointing_device.click_object(element)

    @autopilot_logging.log_action(logger.info)
    def _find_element(self, object_name):
        self._scroll_to_top()
        while not self.atYEnd:
            containers = self._get_containers()
            self._swipe_to_show_more_below(containers)
            try:
                return self.select_single(objectName=object_name)
            except dbus.StateNotFoundError:
                pass
        raise ToolkitEmulatorException(
            'List element with objectName "{}" not found.'.format(object_name))

    def _is_element_clickable(self, object_name):
        child = self.select_single(objectName=object_name)
        containers = self._get_containers()
        return self._is_child_visible(child, containers)


class Empty(UbuntuUIToolkitEmulatorBase):
    """Base class to emulate swipe to delete."""

    def exists(self):
        try:
            return self.implicitHeight > 0
        except dbus.StateNotFoundError:
            return False

    def _get_confirm_button(self):
        return self.select_single(
            'QQuickItem', objectName='confirmRemovalDialog')

    @autopilot_logging.log_action(logger.info)
    def swipe_to_delete(self, direction='right'):
        """Swipe the item in a specific direction."""
        if self.removable:
            self._drag_pointing_device_to_delete(direction)
            if self.confirmRemoval:
                self.waitingConfirmationForRemoval.wait_for(True)
            else:
                self._wait_until_deleted()
        else:
            raise ToolkitEmulatorException(
                'The item "{0}" is not removable'.format(self.objectName))

    def _drag_pointing_device_to_delete(self, direction):
        x, y, w, h = self.globalRect
        tx = x + (w // 8)
        ty = y + (h // 2)

        if direction == 'right':
            self.pointing_device.drag(tx, ty, w, ty)
        elif direction == 'left':
            self.pointing_device.drag(w - (w*0.1), ty, x, ty)
        else:
            raise ToolkitEmulatorException(
                'Invalid direction "{0}" used on swipe to delete function'
                .format(direction))

    def _wait_until_deleted(self):
        try:
            # The item was hidden.
            self.implicitHeight.wait_for(0)
        except dbus.StateNotFoundError:
            # The item was destroyed.
            pass

    @autopilot_logging.log_action(logger.info)
    def confirm_removal(self):
        """Comfirm item removal if this was already swiped."""
        if self.waitingConfirmationForRemoval:
            deleteButton = self._get_confirm_button()
            self.pointing_device.click_object(deleteButton)
            self._wait_until_deleted()
        else:
            raise ToolkitEmulatorException(
                'The item "{0}" is not waiting for removal confirmation'.
                format(self.objectName))


class Base(Empty):
    pass


class Standard(Empty):
    pass


class ItemSelector(Empty):
    pass


class SingleControl(Empty):
    pass


class MultiValue(Base):
    pass


class SingleValue(Base):
    pass


class Subtitled(Base):
    pass


class ComposerSheet(UbuntuUIToolkitEmulatorBase):
    """ComposerSheet Autopilot emulator."""

    def __init__(self, *args):
        super(ComposerSheet, self).__init__(*args)

    @autopilot_logging.log_action(logger.info)
    def confirm(self):
        """Confirm the composer sheet."""
        button = self.select_single('Button', objectName='confirmButton')
        self.pointing_device.click_object(button)
        self.wait_until_destroyed()

    @autopilot_logging.log_action(logger.info)
    def cancel(self):
        """Cancel the composer sheet."""
        button = self.select_single('Button', objectName='cancelButton')
        self.pointing_device.click_object(button)
        self.wait_until_destroyed()


class Application():
    """App class"""
    local_location = ""
    installed_location = ""
    click_package = ""
    app = None
    test_type = None

    def __init__(
        self,
        test_obj,
        local_location="",
        installed_location="",
        click_package=""
    ):
        """Constructor

        :param test_obj: An AutopilotTestCase object.

        :param local_location: Relative path to application. Optional.

        :param installed_location: System path to application. Optional.

        :param click_package: Click package name. Optional.
        """
        self.test_obj = test_obj
        self.local_location = local_location
        self.installed_location = self.installed_location
        self.click_package = click_package
        ## TODO: Check that at least local, installed or click package
        ## argument is passed.

    def launch(self):
        """Launches the application"""
        launch, self.test_type = self.setup_environment()
        launch()

    def setup_environment(self):
        """Selects the way to launch the application"""
        if os.path.exists(self.local_location):
            logger.debug("Running via local installation")
            launch = self.launch_test_local
            test_type = 'local'
        elif os.path.exists(self.installed_location):
            logger.debug("Running via installed debian package")
            launch = self.launch_test_installed
            test_type = 'deb'
        else:
            logger.debug("Running via click package")
            launch = self.launch_test_click
            test_type = 'click'
        return launch, test_type

    def launch_test_local(self):
        """Launch the application using a relative path"""
        self.app = self.test_obj.launch_test_application(
            base.get_qmlscene_launch_command(),
            self.local_location,
            app_type='qt',
            emulator_base=UbuntuUIToolkitEmulatorBase)

    def launch_test_installed(self):
        """Launch the application using the system path"""
        self.app = self.test_obj.launch_test_application(
            base.get_qmlscene_launch_command(),
            self.installed_location,
            app_type='qt',
            emulator_base=UbuntuUIToolkitEmulatorBase)

    def launch_test_click(self):
        """Launch the application using click package"""
        self.app = self.test_obj.launch_click_package(
            self.click_package,
            emulator_base=UbuntuUIToolkitEmulatorBase)

    @property
    def main_view(self):
        """Return MainView object"""
        return self.app.select_single(MainView)