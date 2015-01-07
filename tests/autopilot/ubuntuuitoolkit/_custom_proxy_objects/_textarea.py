# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
#
# Copyright (C) 2014 Canonical Ltd.
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

from ubuntuuitoolkit._custom_proxy_objects import _textfield


class TextArea(_textfield.TextField):
    """TextArea autopilot emulator."""

    def clear(self):
        """Clear the text area."""
        if not self.is_empty():
            self._clear_with_keys()
            self.text.wait_for('')

    def _go_to_end(self):
        # We override this because the text areas can have more than one line.
        # XXX Here we are cheating because the on-screen keyboard doesn't have
        # CTRL nor END keys. --elopio - 2014-08-20
        self.keyboard.press_and_release('Ctrl+End')
