/*  This file is part of the KDE project
    Copyright (C) 2009 Jaroslav Reznik <jreznik@redhat.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "config.h"

#include <KAboutData>
#include <KLocalizedString>
#include <KCrash>
#include <KDBusService>

#include <QSessionManager>

#include "policykitkde.h"

int main(int argc, char *argv[])
{
    KLocalizedString::setApplicationDomain("polkit-kde-authentication-agent-1");
    KAboutData aboutData("polkit-kde-authentication-agent-1", i18n("PolicyKit1 KDE Agent"), POLKIT_KDE_1_VERSION);
    aboutData.addLicense(KAboutLicense::GPL);
    aboutData.addCredit(i18n("(c) 2009 Red Hat, Inc."));
    aboutData.addAuthor(i18n("Lukáš Tinkl"), i18n("Maintainer"), "ltinkl@redhat.com");
    aboutData.addAuthor(i18n("Jaroslav Reznik"), i18n("Former maintainer"), "jreznik@redhat.com");
    aboutData.setProductName("policykit-kde/polkit-kde-authentication-agent-1");

    KAboutData::setApplicationData(aboutData);

    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationName("polkit.kde.authentication.agent1");
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    KCrash::setFlags(KCrash::AutoRestart);

    PolicyKitKDE agent(argc, argv);
    KDBusService service(KDBusService::Unique);

    auto disableSessionManagement = [](QSessionManager &sm) {
        sm.setRestartHint(QSessionManager::RestartNever);
    };

    QObject::connect(&agent, &QGuiApplication::commitDataRequest, disableSessionManagement);
    QObject::connect(&agent, &QGuiApplication::saveStateRequest, disableSessionManagement);

    agent.exec();
}
