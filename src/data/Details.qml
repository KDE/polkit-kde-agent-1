// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.14 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import QtQuick.Templates 2.10 as T

Kirigami.FormLayout {
    QQC2.Label {
        opacity: 0.8
        text: context.details.description
        wrapMode: Text.Wrap
        textFormat: Text.PlainText

        Kirigami.FormData.label: i18nc("label for reason", "What the app is doing:")
    }
    QQC2.Label {
        opacity: 0.8
        text: context.details.message
        wrapMode: Text.Wrap
        textFormat: Text.PlainText

        Kirigami.FormData.label: i18nc("label for reason", "Why they want to do it:")
    }
    QQC2.Label {
        opacity: 0.8
        text: context.details.vendor
        wrapMode: Text.Wrap
        textFormat: Text.PlainText

        Kirigami.FormData.label: i18nc("label for reason", "Who the app is from:")
    }
}