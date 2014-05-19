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

from autopilot import logging as autopilot_logging

from ubuntuuitoolkit._custom_proxy_objects import _common


logger = logging.getLogger(__name__)


# Containers helpers.

def _get_visible_container_top(containers):
    containers_top = [container.globalRect.y for container in containers]
    return max(containers_top)


def _get_visible_container_bottom(containers):
    containers_bottom = [
        container.globalRect.y + container.globalRect.height
        for container in containers if container.globalRect.height > 0]
    return min(containers_bottom)


class Flickable(_common.UbuntuUIToolkitCustomProxyObjectBase):

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

        raise _common.ToolkitException("Couldn't find the top-most container.")

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
            raise _common.ToolkitException(
                "Can't swipe more, we are already at the top of the "
                "container.")
        else:
            self._swipe_to_show_more('above', containers)

    @autopilot_logging.log_action(logger.info)
    def _swipe_to_show_more_below(self, containers):
        if self.atYEnd:
            raise _common.ToolkitException(
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
            raise _common.ToolkitException(
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
