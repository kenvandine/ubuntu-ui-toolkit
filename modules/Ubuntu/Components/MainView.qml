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
// FIXME: When a module contains QML, C++ and JavaScript elements exported,
// we need to use named imports otherwise namespace collision is reported
// by the QML engine. As workaround, we use Theming named import.
// Bug to watch: https://bugreports.qt-project.org/browse/QTBUG-27645
import Ubuntu.Components 0.1 as Theming

/*!
    \qmltype MainView
    \inqmlmodule Ubuntu.Components 0.1
    \ingroup ubuntu
    \brief The root Item for all applications. It automatically adds a header
        and toolbar for its contents.

    Examples:
    \qml
        MainView {
            Page {
                title: "Header text"
                Button {
                    anchors.centerIn: parent
                    text: "Click me"
                }
            }
        }
    \endqml

    Header and toolbar contents are automatically taken from \l Page, \l Tabs and \l PageStack.
    Only one MainView must be used per application, and it should be the root Item of the application.
*/
PageTreeNode {
    id: mainView

    /*!
      \preliminary
      The property holds the application's name, which must be the same as the
      desktop file's name.
      */
    property string applicationName

    // FIXME: Make sure that the theming is only in the background, and the delegate
    //  should not occlude contents of the MainView. When making changes here, make
    //  sure that bug https://bugs.launchpad.net/manhattan/+bug/1124076 does not come back.
    Item {
        id: background
        Theming.ItemStyle.class: "mainview"
        anchors.fill: parent
    }

    // clip if the MainView is not fullscreen
    clip: true

    /*!
      MainView is active by default.
     */
    active: true

    /*!
      \internal
      Use default property to ensure children added do not draw over the toolbar.
     */
    default property alias contentsItem: contents.data
    Item {
        id: contents
        anchors.fill: parent
    }

    /*!
      \deprecated
      The tools of the main view's toolbar.
      This property is deprecated. Pages will now automatically update the toolbar when activated.
     */
    property ToolbarActions tools: null
    onToolsChanged: print("MainView.tools property was deprecated. "+
                          "Pages will automatically update the toolbar when activated. "+
                          "See CHANGES file, and use toolbar.tools instead when needed.");

    /*!
      The header of the MainView. Can be used to obtain the height of the header
      in \l Page to determine the area for the \l Page to fill.
     */
    header: headerItem
    Header {
        id: headerItem
    }

    toolbar: toolbarItem
    Toolbar {
        id: toolbarItem
        Component.onCompleted: toolbarItem.state = "spread"
    }

    MouseArea {
        id: toolbarMouseArea
        anchors.fill: parent
        enabled: true
        z: 100000

        Timer {
            id: delayAfterToolsChange
            interval: 250
            onTriggered: toolbarMouseArea.enabled = true
        }
        Timer {
            id: delayAfterClick
            interval: 250
            onTriggered: {
                if (!toolbarItem.lock) toolbarItem.state = ""
            }
        }

        onPressed: {
            mouse.accepted = false;
            toolbarMouseArea.enabled = false
            delayAfterClick.restart();
        }
        Connections {
            target: toolbarItem
            onToolsChanged: {
                delayAfterClick.stop()
                if (toolbarItem.tools.__hasVisibleActions()) toolbarItem.state = "spread"
                delayAfterToolsChange.restart()
            }
        }
    }

    /*! \internal */
    property QtObject __hud: null

    /*! \internal */
    onApplicationNameChanged: {
        if (applicationName !== "") {
            var component = Qt.createComponent(Qt.resolvedUrl("HudIntegration.qml"));
            if (component)
                __hud = component.createObject(mainView, {"applicationIdentifier": applicationName});
        } else if (__hud)
            __hud.destroy();
    }
}
