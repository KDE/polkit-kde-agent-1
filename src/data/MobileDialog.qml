// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.14 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import QtQuick.Templates 2.10 as T

QQC2.ApplicationWindow {
    id: rootWindow

    visible: true

    title: "⠀"
    visibility: Window.FullScreen
    color: Qt.rgba(0, 0, 0, 0.5)

    property bool normalClosing: false
    property bool showingDetails: false

    onClosing: if (!normalClosing) context.cancel()

    Connections {
        target: context
        function onComplete() {
            normalClosing = true
            rootWindow.close()
        }
    }

    T.Control {
        id: mainContent

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        background: Kirigami.ShadowedRectangle {
            corners.topLeftRadius: 10
            corners.topRightRadius: 10
            color: Kirigami.Theme.backgroundColor
        }

        implicitWidth: Math.max(
            implicitBackgroundWidth + leftInset + rightInset,
            implicitContentWidth + leftPadding + rightPadding)

        implicitHeight: Math.max(
            implicitBackgroundHeight + topInset + bottomInset,
            implicitContentHeight + topPadding + bottomPadding)

        padding: Kirigami.Units.gridUnit
        contentItem: ColumnLayout {
            spacing: Kirigami.Units.gridUnit

            Item { Layout.fillHeight: true }

            AvatarRow {}

            PasswordRow { showOK: true }

            Details { visible: rootWindow.showingDetails }

            Item { Layout.fillHeight: true }

            QQC2.Button {
                id: detailsButton

                text: rootWindow.showingDetails ? i18n("Hide Who/What/Why") : i18n("Show Who/What/Why")
                icon.name: "view-more-symbolic"
                onClicked: rootWindow.showingDetails = !rootWindow.showingDetails

                Layout.alignment: Qt.AlignRight
            }
        }
    }
}