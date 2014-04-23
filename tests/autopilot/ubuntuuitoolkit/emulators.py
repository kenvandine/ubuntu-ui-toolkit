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

import logging
from distutils import version

import autopilot
from autopilot import (
    input,
    logging as autopilot_logging,
    platform
)
from autopilot.introspection import dbus


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

    @autopilot_logging.log_action(logger.info)
    def switch_to_tab_by_index(self, index):
        """Open a tab.

        :parameter index: The index of the tab to open.
        :return: The newly opened tab.
        :raise ToolkitEmulatorException: If the tab index is out of range.

        """
        logger.debug('Switch to tab with index {0}.'.format(index))
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
        toolbar = self.open_toolbar()
        toolbar.click_back_button()


class Header(UbuntuUIToolkitEmulatorBase):
    """Header Autopilot emulator."""

    def __init__(self, *args):
        super(Header, self).__init__(*args)
        self.pointing_device = get_pointing_device()

    def _get_animating(self):
        tab_bar_style = self.select_single('TabBarStyle')
        return tab_bar_style.animating

    @autopilot_logging.log_action(logger.info)
    def switch_to_next_tab(self):
        """Open the next tab.

        :raise ToolkitEmulatorException: If the main view has no tabs.

        """
        try:
            tab_bar = self.select_single(TabBar)
        except dbus.StateNotFoundError:
            raise ToolkitEmulatorException(_NO_TABS_ERROR)
        tab_bar.switch_to_next_tab()
        self._get_animating().wait_for(False)


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
