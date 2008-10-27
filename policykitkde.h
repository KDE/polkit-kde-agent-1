#ifndef POLICYKITKDE_H
#define POLICYKITKDE_H

/*  This file is part of the KDE project
    Copyright (C) 2007-2008 Gökçen Eraslan <gokcen@pardus.org.tr>
    Copyright (C) 2008 Dirk Mueller <mueller@kde.org>

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

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QSocketNotifier>
#include <QtDBus/QDBusContext>

#include <polkit/polkit.h>

class PolicyKitKDE : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.PolicyKit.AuthenticationAgent")

public:
    PolicyKitKDE(QObject* parent=0L);
    ~PolicyKitKDE();

public Q_SLOTS:
    bool ObtainAuthorization(const QString& action_id, uint xid, uint pid);

private Q_SLOTS:
    void watchActivated(int fd);

private:
    PolKitContext *m_context;
    bool inProgress;

    static PolicyKitKDE* m_self;

    QMap<int, QSocketNotifier*> m_watches;

    static int polkit_add_watch(PolKitContext *context, int fd);
    static void polkit_remove_watch(PolKitContext *context, int fd);
    static void polkit_watch_have_data(PolKitContext *context, int fd);
    static void polkit_config_changed(PolKitContext* context, void* );
};

#endif
