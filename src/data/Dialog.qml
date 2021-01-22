// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.14 as Kirigami
import QtQuick.Controls 2.10 as QQC2

QQC2.ApplicationWindow {
    id: rootWindow

    visible: true
    visibility: Kirigami.Settings.isMobile ? Window.FullScreen : Window.Windowed

    title: "⠀"
    flags: Qt.CustomizeWindowHint | Qt.Dialog | Qt.WindowTitleHint

    width: 500
    height: 600

    property bool normalClosing: false

    color: Kirigami.Settings.isMobile ? Qt.rgba(0,0,0,0.5) : Kirigami.Theme.backgroundColor
    onClosing: if (!normalClosing) context.cancel()

    Connections {
        target: context
        onComplete: {
            normalClosing = true
            rootWindow.close()
        }
    }

    Kirigami.Separator {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        // Put this here since the rootWindow isn't an Item and therefore has no states
        states: State {
            when: !Kirigami.Settings.isMobile
            PropertyChanges {
                target: rootWindow

                maximumWidth: 500
                minimumWidth: 500
                maximumHeight: 600
                minimumHeight: 600
            }
        }
    }

    Rectangle {
        visible: Kirigami.Settings.isMobile
        radius: 10

        anchors {
            bottom: parent.bottom
            bottomMargin: -10
            left: parent.left
            right: parent.right
            top: colView.top
            topMargin: -Kirigami.Units.gridUnit
        }

        color: Kirigami.Theme.backgroundColor
    }

    ColumnLayout {
        id: colView
        anchors {
            centerIn: Kirigami.Settings.isMobile ? undefined : parent

            left: parent.left
            right: parent.right
            leftMargin: Kirigami.Settings.isMobile ? Kirigami.Units.gridUnit : 0
            rightMargin: Kirigami.Settings.isMobile ? Kirigami.Units.gridUnit : 0

            bottom: Kirigami.Settings.isMobile ? parent.bottom : undefined
            bottomMargin: Kirigami.Settings.isMobile ? Kirigami.Units.gridUnit*4 : 0
        }

        Kirigami.Heading {
            text: i18n("Authentication Required")
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap

            Layout.fillWidth: true
        }

        Kirigami.Heading {
            text: context.details.otherMessage
            level: 5
            opacity: 0.8
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap

            Layout.fillWidth: true
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: Kirigami.Units.gridUnit
            Layout.bottomMargin: Kirigami.Units.gridUnit

            Kirigami.Avatar {
                source: context.currentAvatar
                name: context.currentUsername

                implicitWidth: Kirigami.Units.gridUnit*5
                implicitHeight: Kirigami.Units.gridUnit*5

                Layout.alignment: Qt.AlignHCenter
            }

            Kirigami.Heading {
                text: context.currentUsername
                level: 2
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap

                Layout.fillWidth: true
            }
        }

        ColumnLayout {
            QQC2.Label {
                font: Kirigami.Theme.smallFont
                text: i18n("Password for %1", context.currentUsername)
            }
            RowLayout {
                Kirigami.PasswordField {
                    id: passField

                    placeholderText: ""
                    onAccepted: context.accept(passField.text)

                    Layout.fillWidth: true
                }
                QQC2.Button {
                    icon.name: Qt.application.direction == Qt.RightToLeft ? "arrow-left" : "arrow-right"

                    onClicked: context.accept(passField.text)

                    QQC2.ToolTip.text: i18n("OK")
                    QQC2.ToolTip.visible: hovered
                    Accessible.description: QQC2.ToolTip.text
                }
            }
            Layout.fillWidth: true
        }

        Kirigami.Heading {
            visible: context.canFingerprint

            text: i18n("You can also use your fingerprint reader to authenticate.")
            level: 5
            opacity: 0.8
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap

            Layout.fillWidth: true
        }
    }

    Kirigami.Separator {
        visible: !expander.childVisible
        anchors {
            left: parent.left
            right: parent.right
            bottom: detailsButton.top
            bottomMargin: Kirigami.Units.largeSpacing
        }
    }

    QQC2.ToolButton {
        anchors {
            left: parent.left
            verticalCenter: detailsButton.verticalCenter
            margins: Kirigami.Units.largeSpacing
        }

        visible: !Kirigami.Settings.isMobile && !expander.childVisible && otherUsersRepeater.count > 1
        text: i18n("Authenticate as another user")
        icon.name: "user-others"
        onClicked: otherUsers.popup()
    }

    QQC2.Menu {
        id: otherUsers
        modal: true

        Repeater {
            id: otherUsersRepeater

            model: context.identityModel

            delegate: QQC2.MenuItem {
                text: realname

                onClicked: context.useIdentity(index)
            }
        }
    }

    QQC2.Button {
        id: detailsButton

        anchors {
            right: parent.right
            bottom: expander.top
            margins: Kirigami.Units.largeSpacing
        }

        text: i18n("Details")
        flat: !expander.childVisible
        icon.name: "view-more-symbolic"
        onClicked: expander.childVisible = !expander.childVisible
    }

    Expandable {
        id: expander

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        QQC2.Control {
            // TODO: figure out why Kirigami.Units isn't working here
            topPadding: 20
            leftPadding: 20
            rightPadding: 20
            bottomPadding: 20
            padding: 20
            anchors.left: parent.left
            anchors.right: parent.right

            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false

            background: Rectangle {
                color: Kirigami.Theme.backgroundColor

                Kirigami.Separator {
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                    }
                }
            }

            contentItem: Kirigami.FormLayout {
                QQC2.Label {
                    opacity: 0.8
                    text: context.details.description
                    wrapMode: Text.Wrap

                    Kirigami.FormData.label: i18n("What the app is doing:")
                }
                QQC2.Label {
                    opacity: 0.8
                    text: context.details.message
                    wrapMode: Text.Wrap

                    Kirigami.FormData.label: i18n("Why they want to do it:")
                }
                QQC2.Label {
                    opacity: 0.8
                    text: context.details.vendor
                    wrapMode: Text.Wrap

                    Kirigami.FormData.label: i18n("Who the app is from:")
                }
            }
        }
    }
}