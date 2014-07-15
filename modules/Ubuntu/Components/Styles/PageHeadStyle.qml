/*
 * Copyright 2014 Canonical Ltd.
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
    \qmltype PageHeadStyle
    \inqmlmodule Ubuntu.Components.Styles 1.1
    \ingroup style-api
    \brief Style API for page header.

    The component defines the style API for the page header component.
  */
Item {
    /*!
      The height of the headercontents, which is the full height of
      the header minus the separators shown at the bottom of it.
     */
    property real contentHeight

    /*!
      The source of the image that separates the header from the contents of a \l MainView.
      The separator will be drawn over the contents.
     */
    property url separatorSource

    /*!
      The source of an additional image attached to the bottom of the separator. The contents
      of the \l MainView will be drawn on top of the separator bottom image.
     */
    property url separatorBottomSource

    /*!
      The size of the title font.
     */
    property string fontSize

    /*!
      The weight of the title font.
     */
    property int fontWeight

    /*!
      The color of the title text.
     */
    property color textColor

    /*!
      Left margin of title text.
     */
    property real textLeftMargin

    /*!
      The number of slots for actions in the header, including the optional
      (custom or automatic) back button in the left side of the header.
      If the number of actions defined is larger than the numer of actions
      specified here, extra actions are put into an overflow.
     */
    property int maximumNumberOfActions
}
