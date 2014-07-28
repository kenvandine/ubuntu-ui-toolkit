/*
 * Copyright 2012-2014 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.2
import Ubuntu.Components 1.1

AbstractButton {
    id: button

    property int iconWidth: units.gu(2)
    property int iconHeight: iconWidth

    width: visible ? units.gu(4) : 0
    height: parent ? parent.height : undefined

    Image {
        id: icon
        anchors {
            centerIn: parent
        }
        width: button.iconWidth
        height: button.iconHeight
        source: button.iconSource
        opacity: button.enabled ? 1.0 : 0.3
        sourceSize {
            width: icon.width
            height: icon.height
        }
    }
}
