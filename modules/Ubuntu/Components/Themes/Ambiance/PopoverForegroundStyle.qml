/*
 * Copyright 2012 Canonical Ltd.
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

import QtQuick 2.0
import Ubuntu.Components 0.1
import QtGraphicalEffects 1.0

Item {
    id: frame
    // styling properties
    property string radius: "medium"
    property color color: Theme.palette.normal.overlay

    property alias contentItem: body

    anchors.fill: parent ? parent : undefined

    Item {
        id: content
        anchors.fill: parent
        anchors.margins: units.dp(4)

        Item {
            id: body
            anchors.fill: parent

            Rectangle {
                id: background
                anchors.fill: parent
                color: frame.color
            }
        }

        clip: true // hide the ShaderEffectSource
        UbuntuShape {
            anchors.fill: parent
            image: effectSource
            radius: frame.radius
        }

        ShaderEffectSource {
            smooth: false // prevent linear interpolation
            id: effectSource
            hideSource: true
            sourceItem: frame.contentItem
            format: ShaderEffectSource.RGBA
            live: true

            // Do not set visible to false because it will leave the FBO empty,
            //  but position the ShaderEffectSource somewhere that it will be clipped
            //  so it is not visible.
            x: width
            width: sourceItem.width
            height: sourceItem.height
        }
    }
    DropShadow {
        anchors.fill: content
        source: content
        radius: units.gu(1)
        samples: 3 * radius
        fast: true
        spread: 0
        transparentBorder: true
        color: Qt.rgba(0, 0, 0, 0.7)
    }
}
