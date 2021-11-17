/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.14 as Kirigami
import org.kde.plasma.workspace.dialogs 1.0 as PWD
import org.kde.polkitkde 1.0

PWD.DesktopSystemDialog
{
    id: desktopPolkit
    title: i18n("Authentication Required")
    property alias password: passwordField.text
    property alias inlineMessageType: inlineMessage.type
    property alias inlineMessageText: inlineMessage.text
    property alias inlineMessageIcon: inlineMessage.icon
    property alias identitiesModel: identitiesCombo.model
    property alias identitiesCurrentIndex: identitiesCombo.currentIndex
    property alias selectedIdentity: identitiesCombo.currentValue
    property var description

    function authenticationFailure()
    {
        inlineMessage.type = Kirigami.InlineMessage.Error;
        inlineMessage.text = i18n("Authentication failure, please try again.");
        passwordField.clear()
        passwordField.enabled = true
        passwordField.focus = true
    }

    ColumnLayout {
        Kirigami.InlineMessage {
            id: inlineMessage
            showCloseButton: true
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing * 2
            visible: text.length !== 0
        }

        QQC2.ComboBox {
            id: identitiesCombo
            Layout.fillWidth: true
            textRole: "display"
            valueRole: "userRole"
            enabled: count > 0
            model: IdentitiesModel {}
        }

        Kirigami.PasswordField {
            id: passwordField
            Layout.fillWidth: true
            onAccepted: desktopPolkit.accept()
        }

        Kirigami.FormLayout {
            visible: details.checked
            QQC2.Label {
                Kirigami.FormData.label: i18n("Action:")
                text: desktopPolkit.description.description
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("ID:")
                text: desktopPolkit.description.actionId
            }
            Kirigami.UrlButton {
                Kirigami.FormData.label: i18n("Vendor:")
                text: desktopPolkit.description.vendorName
                url: desktopPolkit.description.vendorUrl
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    actions: [
        Kirigami.Action {
            id: details
            text: i18n("Details")
            checkable: true
            checked: false
        }
    ]

    dialogButtonBox.standardButtons: QQC2.DialogButtonBox.Ok | QQC2.DialogButtonBox.Cancel
}
