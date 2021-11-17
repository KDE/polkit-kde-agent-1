// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.14 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import QtQuick.Templates 2.10 as T

ColumnLayout {
    id: passRow

    required property bool showOK
    function submit() {
        context.accept(passField.text)
    }

    QQC2.Label {
        font: Kirigami.Theme.smallFont
        text: i18n("Password for %1", context.currentUsername)
    }
    RowLayout {
        Kirigami.PasswordField {
            id: passField

            placeholderText: ""
            onAccepted: passRow.submit()

            Layout.fillWidth: true
        }
        QQC2.Button {
            visible: passRow.showOK
            icon.name: Qt.application.direction == Qt.RightToLeft ? "arrow-left" : "arrow-right"

            onClicked: passRow.submit()

            QQC2.ToolTip.text: i18n("OK")
            QQC2.ToolTip.visible: hovered
            Accessible.description: QQC2.ToolTip.text
        }
        Layout.fillWidth: true
    }
    Layout.fillWidth: true

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
