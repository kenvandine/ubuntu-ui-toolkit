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
 *
 * Author: Christian Dywan <christian.dywan@canonical.com>
 */

import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

Rectangle {
    property ListModel model: null
    anchors.fill: parent

    ListView {
        id: emailView
        model: parent.model
        width: parent.width
        height: parent.height
        delegate: ListItem.Subtitled {
            text: subject + " <em>" + when + "</em>"
            subText: preview
            icon: Icon {
                name: "search"
                width: units.gu(3)
                height: units.gu(5)
                visible: starred
            }
        }
    }
}

