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
import Ubuntu.Components 1.1

// frame
// FIXME: stabilize API
Item {
    id: visuals
    // style properties
    property url iconSource: "artwork/clear.svg"
    // FIXME: needs type checking in themes to define the proper type to be used
    // if color type is used, alpha value gets lost

    property color color: (styledItem.focus || styledItem.highlighted) ? Theme.palette.selected.fieldText : Theme.palette.normal.fieldText
    /*!
      Background fill color
      */
    property color backgroundColor: (styledItem.focus || styledItem.highlighted) ? Theme.palette.selected.field : Theme.palette.normal.field
    property color errorColor: UbuntuColors.orange

    /*!
      Spacing between the frame and the text editor area
      */
    property real frameSpacing: units.gu(1)
    property real overlaySpacing: frameSpacing / 2

    /*!
      The following properties define the name of the style components declaring
      the styles for the main and the selection cursors. All cursors must defive
      from TextCursorStyle.
      */
    property string mainCursorStyle: "TextCursorStyle.qml"
    property string selectionStartCursorStyle: "TextSelectionStartCursorStyle.qml"
    property string selectionEndCursorStyle: "TextSelectionEndCursorStyle.qml"

    // style body
    anchors.fill: parent
    objectName: "textarea_style"

    z: -1

    /*!
      Text input background
      */
    property Component background: UbuntuShape {
        property bool error: (styledItem.hasOwnProperty("errorHighlight") && styledItem.errorHighlight && !styledItem.acceptableInput)
        onErrorChanged: (error) ? visuals.errorColor : visuals.backgroundColor;
        color: visuals.backgroundColor;
        anchors.fill: parent
    }

    Loader {
        id: backgroundLoader
        sourceComponent: background
        anchors.fill: parent
    }
}
