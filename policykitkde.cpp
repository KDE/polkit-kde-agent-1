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

#include "policykitkde.h"

#include <KDebug>
#include <PolkitQt1/Subject>

PolicyKitKDE::PolicyKitKDE()
        : m_listener(new PolicyKitListener(this))
{
    setQuitOnLastWindowClosed(false);

    PolkitQt1::UnixSessionSubject session(getpid());

    bool result = m_listener->registerListener(session, "/org/kde/PolicyKit1/AuthenticationAgent");

    kDebug() << result;

    if (!result) {
        kDebug() << "Couldn't register listener!";
        exit(1);
    }
}

PolicyKitKDE::~PolicyKitKDE()
{
    m_listener->deleteLater();
}
