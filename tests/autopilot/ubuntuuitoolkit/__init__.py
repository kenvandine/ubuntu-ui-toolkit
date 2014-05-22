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

"""Ubuntu UI Toolkit Autopilot tests and helpers."""


__all__ = [
    'base',
    'check_autopilot_version',
    'CheckBox',
    'environment',
    'emulators',
    'fixture_setup',
    'get_keyboard',
    'get_pointing_device',
    'Header',
    'listitems',
    'MainView',
    'OptionSelector',
    'popups',
    'QQuickFlickable',
    'QQuickListView',
    'TabBar',
    'Tabs',
    'tests',
    'TextField',
    'Toolbar',
    'ToolkitException',
    'UbuntuUIToolkitCustomProxyObjectBase',
]


from ubuntuuitoolkit import (
    base,
    emulators,
    environment,
    fixture_setup,
    tests,
)
from ubuntuuitoolkit._custom_proxy_objects import (
    check_autopilot_version,
    CheckBox,
    get_keyboard,
    get_pointing_device,
    Header,
    listitems,
    MainView,
    OptionSelector,
    popups,
    QQuickFlickable,
    QQuickListView,
    TabBar,
    Tabs,
    TextField,
    Toolbar,
    ToolkitException,
    UbuntuUIToolkitCustomProxyObjectBase,
)
