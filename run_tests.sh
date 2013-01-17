#!/bin/bash
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

cd tests/autopilot

echo running with arg: $1

if [ "$1" == "" ]; then
	autopilot run ubuntu-ui-toolkit
else
	autopilot run -o ../../$1 -f xml -r -rd ../../ ubuntu-ui-toolkit
fi

exit 0

