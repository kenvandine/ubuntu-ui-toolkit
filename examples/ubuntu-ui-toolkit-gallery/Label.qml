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
import Ubuntu.Components 1.0

Template {
    objectName: "labelsTemplate"

    TemplateSection {
        className: "Label"

        Column {
            spacing: units.gu(2)

            Label {
                fontSize: "xx-small"
                text: "xx-small"
            }
            Label {
                fontSize: "x-small"
                text: "x-small"
            }
            Label {
                fontSize: "small"
                text: "small"
            }
            Label {
                fontSize: "medium"
                text: "medium"
            }
            Label {
                fontSize: "large"
                text: "large"
            }
            Label {
                fontSize: "x-large"
                text: "x-large"
            }
        }
    }
}
