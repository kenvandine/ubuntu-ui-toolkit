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
import Ubuntu.Components 0.1

Row {
    id: row

    /*
      Reference to the main composit component holding this row.
      */
    property Item mainComponent

    /*
      The model populating the row.
      */
    property alias model: rowRepeater.model

    /*
      Picker label margins
      */
    property real margins: units.gu(1.5)

    objectName: "PickerRow_Positioner";

    Repeater {
        id: rowRepeater
        Picker {
            id: unitPicker
            objectName: "PickerRow_" + pickerName
            model: pickerModel
            enabled: pickerModel.count > 1
            circular: pickerModel.circular
            live: false
            width: pickerModel.pickerWidth
            height: parent.height

            style: Rectangle {
                anchors.fill: parent
                color: (unitPicker.Positioner.index % 2) ? Qt.rgba(0, 0, 0, 0.03) : Qt.rgba(0, 0, 0, 0.07)
            }
            delegate: PickerDelegate {
                Label {
                    objectName: "PickerRow_PickerLabel"
                    text: pickerModel.text(modelData)
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
                Component.onCompleted: {
                    if (pickerModel && pickerModel.autoExtend && (index === (pickerModel.count - 1))) {
                        pickerModel.extend(modelData + 1);
                    }
                }
            }

            onSelectedIndexChanged: {
                if (pickerModel && !pickerModel.resetting) {
                    mainComponent.date = pickerModel.dateFromIndex(selectedIndex);
                    pickerModel.syncModels();
                }
            }

            /*
              Resets the Picker model and updates the new format limits.
              */
            function resetPicker() {
                pickerModel.reset();
                pickerModel.resetLimits(textSizer, margins);
                pickerModel.resetCompleted();
                selectedIndex = pickerModel.indexOf();
            }

            Component.onCompleted: {
                // update model with the item instance
                pickerModel.pickerItem = unitPicker;
            }
        }
    }
}
