// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.14 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import QtQuick.Templates 2.10 as T

RowLayout {
    Kirigami.Avatar {
        source: context.currentAvatar
        name: context.currentUsername

        implicitWidth: Kirigami.Units.gridUnit*2
        implicitHeight: Kirigami.Units.gridUnit*2

        Layout.alignment: Qt.AlignVCenter
    }
    ColumnLayout {
        spacing: 4

        Kirigami.Heading {
            text: i18n("Authentication Required")
            font.bold: true
            wrapMode: Text.Wrap

            Layout.fillWidth: true
        }

        Kirigami.Heading {
            text: context.details.otherMessage
            level: 5
            opacity: 0.8
            wrapMode: Text.Wrap

            Layout.fillWidth: true
        }
    }
}