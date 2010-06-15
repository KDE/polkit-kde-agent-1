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

#include <KCmdLineArgs>
#include <KAboutData>
#include <KLocale>

#include "policykitkde.h"

int main(int argc, char *argv[])
{
    KAboutData aboutData("policykit1-kde", "polkit-kde-authentication-agent-1", ki18n("PolicyKit1-KDE"), "0.1",
                         ki18n("PolicyKit1-KDE"), KAboutData::License_GPL,
                         ki18n("(c) 2009 Red Hat, Inc."));
    aboutData.addAuthor(ki18n("Jaroslav Reznik"), ki18n("Maintainer"), "jreznik@redhat.com");
    aboutData.setProductName("policykit-kde/polkit-kde-authentication-agent-1");

    KCmdLineArgs::init(argc, argv, &aboutData);

    if (!PolicyKitKDE::start()) {
        qWarning("PolicyKitKDE is already running!\n");
        return 0;
    }

    PolicyKitKDE agent;
    agent.disableSessionManagement();
    agent.exec();
}
