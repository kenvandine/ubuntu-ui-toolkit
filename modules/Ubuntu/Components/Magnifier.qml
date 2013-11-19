/*
 * Copyright 2013 Canonical Ltd.
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

/*!
    \qmltype Magnifier
    \inqmlmodule Ubuntu.Components 0.1
    \ingroup ubuntu
    \brief TODO. Probably    this component will be internal after testing it.
*/
Item {
    id: magnifier
    property alias sourceItem: effectSource.sourceItem
    property alias scaleFactor: effect.scaleFactor

    ShaderEffectSource {
        id: effectSource
        anchors.fill: parent
        visible: false
        sourceItem: magnifier.parent
        width: sourceItem.width
        height: sourceItem.height
        live: true
        smooth: true
    }

    ShaderEffect {
        id: effect
        anchors.fill: parent
        property variant source: effectSource

        // FIXME: I don't know where the factor 2 comes from
        property real targetX: magnifier.mapToItem(sourceItem, magnifier.x, magnifier.y).x / sourceItem.width / 2
        property real targetY: magnifier.mapToItem(sourceItem, magnifier.x, magnifier.y).y / sourceItem.height / 2
        property real targetWidth: magnifier.width / sourceItem.width
        property real targetHeight: magnifier.height / sourceItem.height

        property real scaleFactor: 1.2

        vertexShader: "
            attribute highp vec4 qt_Vertex;
            attribute highp vec2 qt_MultiTexCoord0;
            uniform highp mat4 qt_Matrix;
            uniform highp float scaleFactor;
            varying highp vec2 qt_TexCoord0;
            void main() {
                qt_TexCoord0 = vec2(0.5 - 1.0 / (2.0 * scaleFactor)) + qt_MultiTexCoord0 / vec2(scaleFactor);
                gl_Position = qt_Matrix * qt_Vertex;
            }";

        fragmentShader: "
            uniform lowp float qt_Opacity;
            varying highp vec2 qt_TexCoord0;
            uniform highp float targetX;
            uniform highp float targetY;
            uniform highp float targetWidth;
            uniform highp float targetHeight;
            uniform sampler2D source;

            void main() {
                vec2 tc = vec2(targetX, targetY) + qt_TexCoord0 * vec2(targetWidth, targetHeight);
                lowp vec4 tex = texture2D(source, tc);
                gl_FragColor = tex.rgba * qt_Opacity;
            }
            "
    }
}
