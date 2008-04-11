/*  This file is part of the KDE project
    Copyright (C) 2007-2008 Gökçen Eraslan <gokcen@pardus.org.tr>

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


#include <qstring.h>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <klocale.h>

#include "authdialog.h"
#include "policykitkde.h"

int main (int argc, char *argv[])
{
    KAboutData aboutData( "policykit-kde", "", ki18n( "PolicyKit-kde" ), "0.1",
                        ki18n( "PolicyKit-kde" ), KAboutData::License_GPL,
                        ki18n( "(c) 2005-2007, TUBITAK - UEKAE" ) );
    aboutData.addAuthor( ki18n("Gökçen Eraslan"), ki18n( "KDE 3 Code" ), "gokcen@pardus.org.tr" );
    aboutData.addAuthor( ki18n("Dirk Müller"), ki18n( "Author" ), "mueller@kde.org" );
    KCmdLineArgs::init( argc, argv, &aboutData );

    KApplication app;

    PolicyKitKDE pkKDE;
    return app.exec();
}
