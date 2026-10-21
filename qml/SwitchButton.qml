/*  This file is part of the KDE project
 *  SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
 *  SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
 *  SPDX-FileCopyrightText: 2026 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.polkitkde

Kirigami.LinkButton {
    id: control

    property IdentitiesModel model: IdentitiesModel { }
    property int currentIndex: -1
    property string currentValue: currentIndex >= 0 ? model.data(model.index(currentIndex, 0), Qt.UserRole) : ""
    property string currentDisplay: currentIndex >= 0 ? model.data(model.index(currentIndex, 0), Qt.DisplayRole) : ""

    property alias count: menu.count

    text: i18nc("@action:button Switches user used for authorization", "Switch…")

    onClicked: {
        menu.popup(0, height)
    }

    QQC2.Menu {
        id: menu
        Repeater {
            model: control.model

            QQC2.MenuItem {
                required property int index
                required property var model

                text: model.display
                checkable: true
                checked: index === control.currentIndex
                autoExclusive: true

                onClicked: {
                    control.currentIndex = index
                }
            }
        }
    }
}
