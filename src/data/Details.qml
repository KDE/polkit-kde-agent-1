// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.10
import QtQuick.Window 2.10
import QtQuick.Layouts 1.10
import org.kde.kirigami 2.14 as Kirigami
import QtQuick.Controls 2.10 as QQC2
import QtQuick.Templates 2.10 as T

/*
    Kirigami's entire schtick is that controls resize to the window.
    In this dialog, we size the window to the controls.
    But oops! you can't have both the controls sizing to window
    and the window sizing to the controls, and trying this results
    in binding loops / layout spazzing.

    Not using a FormLayout fixes this, so please don't port to a Kirigami FormLayout.
*/
GridLayout {
    columns: Kirigami.Settings.isMobile ? 1 : 2

    component PlainLabel : QQC2.Label {
        textFormat: Text.PlainText
    }
    component KeyLabel : PlainLabel {
        Layout.alignment: Kirigami.Settings.isMobile ? Qt.AlignLeft : Qt.AlignRight
    }
    component ValLabel : PlainLabel {
        opacity: 0.8
        wrapMode: Text.Wrap
    }

    KeyLabel {
        text: i18nc("label for reason", "What the app is doing:")
    }
    ValLabel {
        text: context.details.description
    }
    KeyLabel {
        text: i18nc("label for reason", "Why they want to do it:")
    }
    ValLabel {
        text: context.details.message
    }
    KeyLabel {
        text: i18nc("label for reason", "Who the app is from:")
    }
    ValLabel {
        text: context.details.vendor
    }
}