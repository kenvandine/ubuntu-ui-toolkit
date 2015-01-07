#!/bin/sh
#
# Copyright 2012 Canonical Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

export QML_IMPORT_PATH=$PWD/modules
export QML2_IMPORT_PATH=$PWD/modules
export UBUNTU_UI_TOOLKIT_THEMES_PATH=$PWD/modules
/sbin/initctl set-env --global QML_IMPORT_PATH=$PWD/modules
/sbin/initctl set-env --global QML2_IMPORT_PATH=$PWD/modules
/sbin/initctl set-env --global UBUNTU_UI_TOOLKIT_THEMES_PATH=$PWD/modules
