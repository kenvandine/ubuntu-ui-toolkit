#!/bin/bash
#
# Copyright 2012, 2013 Canonical Ltd.
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

cd tests/autopilot

echo running with arg: $1

UBUNTU_UI_TOOLKIT_AUTOPILOT_FROM_SOURCE=1
if [ "$1" == "" ]; then
	python3 -m autopilot.run run ubuntuuitoolkit
else
	python3 -m autopilot.run run -o ../../$1 -f xml -r -rd ../../ ubuntuuitoolkit
fi

exit 0

