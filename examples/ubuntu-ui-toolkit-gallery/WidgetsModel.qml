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

ListModel {
    // Already in design guidelines
    ListElement {
        objectName: "navigationElement"
        label: "Navigation"
        source: "Navigation.qml"
    }
    ListElement {
        objectName: "stylesElement"
        label: "Styles"
        source: "Styles.qml"
    }
    ListElement {
        objectName: "colorsElement"
        label: "Colors"
        source: "Colors.qml"
    }
    ListElement {
        objectName: "togglesElement"
        label: "Toggles"
        source: "Toggles.qml"
    }
    ListElement {
        objectName: "toolbarElement"
        label: "Toolbar"
        source: "Toolbar.qml"
    }
    ListElement {
        objectName: "buttonsElement"
        label: "Buttons"
        source: "Buttons.qml"
    }
    ListElement {
        objectName: "slidersElement"
        label: "Slider"
        source: "Sliders.qml"
    }
    ListElement {
        objectName: "textinputsElement"
        label: "Text Field"
        source: "TextInputs.qml"
    }

    ListElement {
        objectName: "optionSelectorsElement"
        label: "Option Selector"
        source: "OptionSelectors.qml"
    }

    // Not in design guidelines yet
    ListElement {
        objectName: "pickersElement"
        label: "Pickers"
        source: "Pickers.qml"
    }
    ListElement {
        objectName: "progressBarsElement"
        label: "Progress and activity"
        source: "ProgressBars.qml"
    }
    ListElement {
        objectName: "ubuntuShapesElement"
        label: "Ubuntu Shape"
        source: "UbuntuShape.qml"
    }
    ListElement {
        objectName: "iconsElement"
        label: "Icons"
        source: "Icons.qml"
    }
    ListElement {
        objectName: "labelsElement"
        label: "Label"
        source: "Label.qml"
    }
    ListElement {
        objectName: "crossFadeImageElement"
        label: "CrossFadeImage"
        source: "CrossFadeImage.qml"
    }

    // Already in design guidelines but should be reordered
    ListElement {
        objectName: "listItemsElement"
        label: "List Items"
        source: "ListItems.qml"
    }
    ListElement {
        objectName: "ubuntuListViewElement"
        label: "Ubuntu ListView"
        source: "UbuntuListViews.qml"
    }

    ListElement {
        objectName: "dialogsElement"
        label: "Dialog"
        source: "Dialog.qml"
    }
    ListElement {
        objectName: "popoversElement"
        label: "Popover"
        source: "Popover.qml"
    }
    ListElement {
        objectName: "sheetsElement"
        label: "Sheet"
        source: "Sheet.qml"
    }

    // Not in design guidelines yet
    ListElement {
        objectName: "animationsElement"
        label: "Animations"
        source: "Animations.qml"
    }
}
