/*  This file is part of the KDE project
    Copyright (C) 2007-2008 Gökçen Eraslan <gokcen@pardus.org.tr>
    Copyright (C) 2008 Dario Freddi <drf54321@gmail.com>

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
#include <KUniqueApplication>
#include <KLocale>

#include "authdialog.h"
#include "policykitkde.h"

int main (int argc, char *argv[])
{
    KAboutData aboutData( "policykit-kde", "", ki18n( "PolicyKit-KDE" ), "0.2",
                        ki18n( "PolicyKit-KDE" ), KAboutData::License_GPL,
                        ki18n( "(c) 2005-2007, TUBITAK - UEKAE" ) );
    aboutData.addAuthor( ki18n("Gökçen Eraslan"), ki18n( "KDE 3 Code" ), "gokcen@pardus.org.tr" );
    aboutData.addAuthor( ki18n("Dirk Müller"), ki18n( "Author" ), "mueller@kde.org" );
    aboutData.addAuthor( ki18n("Luboš Luňák"), ki18n( "Developer" ), "l.lunak@kde.org" );
    KCmdLineArgs::init( argc, argv, &aboutData );

    if (!KUniqueApplication::start()) {
        fprintf(stderr, "PolicyKitKDE is already running!\n");
        return 0;
    }

    KUniqueApplication a;
    a.setQuitOnLastWindowClosed(false);
    a.disableSessionManagement();

    PolicyKitKDE::instance();

    return a.exec();
}
