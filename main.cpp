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

#include <KCmdLineArgs>
#include <K4AboutData>
#include <KLocale>
#include <KCrash>

#include "policykitkde.h"

int main(int argc, char *argv[])
{
    K4AboutData aboutData("polkit-kde-authentication-agent-1", "polkit-kde-authentication-agent-1", ki18n("PolicyKit1-KDE"), POLKIT_KDE_1_VERSION,
                          ki18n("PolicyKit1-KDE"), K4AboutData::License_GPL,
                          ki18n("(c) 2009 Red Hat, Inc."));
    aboutData.addAuthor(ki18n("Lukáš Tinkl"), ki18n("Maintainer"), "ltinkl@redhat.com");
    aboutData.addAuthor(ki18n("Jaroslav Reznik"), ki18n("Former maintainer"), "jreznik@redhat.com");
    aboutData.setProductName("policykit-kde/polkit-kde-authentication-agent-1");

    KCmdLineArgs::init(argc, argv, &aboutData);

    if (!PolicyKitKDE::start()) {
        qWarning("PolicyKitKDE is already running!\n");
        return 0;
    }

    KCrash::setFlags(KCrash::AutoRestart);

    PolicyKitKDE agent;
    agent.disableSessionManagement();
    agent.exec();
}
