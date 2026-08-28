/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2026 User8395 <therealuser8395@proton.me>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami 2.19 as Kirigami
import org.kde.bigscreen as Bigscreen
import org.kde.polkitkde 1.0

Window {
    id: root
    title: i18n("Authentication Required")
    color: "transparent"
    flags: Qt.WindowStaysOnTopHint | Qt.FramelessWindowHint

    property alias password: passwordField.text
    property alias inlineMessageType: inlineMessage.type
    property alias inlineMessageText: inlineMessage.text
    property alias inlineMessageIcon: inlineMessage.icon 
    property alias identitiesModel: identitiesCombo.model
    property alias identitiesCurrentIndex: identitiesCombo.currentIndex
    property alias selectedIdentity: identitiesCombo.currentValue

    property bool waitingForAuthentication: false

    // passed in by QuickAuthDialog.cpp
    property alias mainText: dialogHeading.text
    property string descriptionString
    property string descriptionActionId
    property string descriptionVendorName
    property string descriptionVendorUrl

    signal accept()
    signal reject()
    signal userSelected()

    onSelectedIdentityChanged: userSelected()

    onAccept: {
        waitingForAuthentication = true;
        if (passwordField.text !== "") {
            passwordField.enabled = false;
        }
    }

    function rejectPassword() {
        passwordField.clear()
        passwordField.enabled = true
        waitingForAuthentication = false
        passwordField.forceActiveFocus()
    }

    function authenticationFailure() {
        inlineMessage.type = Kirigami.MessageType.Error;
        inlineMessage.text = i18n("Authentication failure, please try again.");
        rejectPassword()
    }

    function request() {
        if (passwordField.text !== "" && waitingForAuthentication) {
            rejectPassword()
        }
    }

    onVisibleChanged: {
        if (visible) {
            dialog.open();
            showMaximized();
        }
    }

    Bigscreen.ComboBoxDelegate {
        id: identitiesCombo
        visible: false
        text: i18n("Change user")
        textRole: "display"
        valueRole: "userRole"
        enabled: count > 0
        model: IdentitiesModel {}
        focusPolicy: Qt.NoFocus
    }

    Bigscreen.Dialog {
        id: dialog

        onAccepted: root.accept()
        onRejected: root.reject()
        onClosed: root.reject()

        contentItem: ColumnLayout {
            QQC2.Label {
                id: dialogHeading
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                font.pixelSize: Bigscreen.Units.headingFontPixelSize
                Layout.bottomMargin: Kirigami.Units.largeSpacing
            }

            QQC2.Label {
                id: dialogAuthUserLabel
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                font.pixelSize: Bigscreen.Units.defaultFontPixelSize
                text: i18n("Authenticating as %1", identitiesCombo.currentText)
                opacity: 0.75
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.largeSpacing
            }

            // TODO: replace this with something that fits Bigscreen
            Kirigami.InlineMessage {
                id: inlineMessage
                Layout.fillWidth: true
                showCloseButton: true
                visible: text.length !== 0
            }

            Bigscreen.TextField {
                id: passwordField
                Layout.fillWidth: true
                echoMode: TextInput.Password
                placeholderText: i18n("Password…")
                onAccepted: root.accept()
                Layout.topMargin: Kirigami.Units.smallSpacing
                KeyNavigation.down: changeUserButton.visible ? changeUserButton : okButton
            }
        }

        footer: QQC2.DialogButtonBox {
            id: dialogFooter
            alignment: Qt.AlignRight
            onActiveFocusChanged: {
                // This DialogButtonBox loves to steal focus when the dialog opens,
                // so snatch it away and give it to the password field,
                // while also allowing the buttons to properly get it.
                if (!changeUserButton.focus && !okButton.focus && !closeButton.focus) {
                    passwordField.forceActiveFocus()
                }
            }

            Bigscreen.Button {
                id: changeUserButton
                icon.name: "user"
                text: i18n("Change user")
                visible: identitiesCombo.count > 1
                onClicked: identitiesCombo.click()
                KeyNavigation.right: okButton
                KeyNavigation.up: passwordField
                KeyNavigation.left: passwordField
            }

            Bigscreen.Button {
                id: okButton
                icon.name: "dialog-ok"
                text: i18n("OK")
                onClicked: accept()
                KeyNavigation.up: passwordField
                KeyNavigation.left: changeUserButton.visible ? changeUserButton : passwordField
                KeyNavigation.right: closeButton
            }

            Bigscreen.Button {
                id: closeButton
                icon.name: "dialog-cancel"
                text: i18n("Cancel")
                onClicked: reject()
                KeyNavigation.up: passwordField
                KeyNavigation.left: okButton
            }
        }
    }
}