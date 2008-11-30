#ifndef POLICYKITKDE_H
#define POLICYKITKDE_H

/*  This file is part of the KDE project
    Copyright (C) 2007-2008 Gökçen Eraslan <gokcen@pardus.org.tr>
    Copyright (C) 2008 Dirk Mueller <mueller@kde.org>
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


#include <QtCore/QMap>
#include <QtCore/QSocketNotifier>
#include <QtDBus/QDBusContext>
#include <QtDBus/QDBusMessage>
#include <QtGui/QWidget>

#include <polkit/polkit.h>
#include <polkit-grant/polkit-grant.h>

enum KeepPassword {
    KeepPasswordNo, KeepPasswordSession, KeepPasswordAlways
};

class AuthDialog;

class PolicyKitKDE : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.PolicyKit.AuthenticationAgent")

public:
    static PolicyKitKDE *instance();

    PolicyKitKDE(QObject* parent = 0L);
    ~PolicyKitKDE();

public Q_SLOTS:
    bool ObtainAuthorization(const QString& action_id, uint xid, uint pid);

private Q_SLOTS:
    void watchActivatedGrant(int fd);
    void watchActivatedContext(int fd);
    void childTerminated(pid_t, int);
    void finishObtainPrivilege();
    void dialogAccepted();
    void dialogCancelled();

private:
    PolKitContext *m_context;
    WId parent_wid;
    AuthDialog* dialog;
    bool inProgress;
    bool cancelled;
    PolKitGrant* grant;
    PolKitCaller* caller;
    PolKitAction* action;
    bool obtainedPrivilege;
    bool requireAdmin;
    KeepPassword keepPassword;
    QDBusMessage mes;

    QMap<int, QSocketNotifier*> m_watches;

    static int add_grant_io_watch(PolKitGrant *grant, int fd);
    static void remove_grant_io_watch(PolKitGrant *grant, int fd);
    static int add_context_io_watch(PolKitContext *context, int fd);
    static void remove_context_io_watch(PolKitContext *context, int fd);
    static int add_child_watch(PolKitGrant* grant, pid_t pid);
    static void remove_child_watch(PolKitGrant* grant, int id);
    static void remove_watch(PolKitGrant* grant, int id);
    static void conversation_type(PolKitGrant* grant, PolKitResult type, void* d);
    static char* conversation_select_admin_user(PolKitGrant* grant, char** users, void* d);
    static char* conversation_pam_prompt_echo_off(PolKitGrant* grant, const char* request, void* d);
    static char* conversation_pam_prompt_echo_on(PolKitGrant* grant, const char* request, void* d);
    static void conversation_pam_error_msg(PolKitGrant* grant, const char* msg, void* d);
    static void conversation_pam_text_info(PolKitGrant* grant, const char* msg, void* d);
    static PolKitResult conversation_override_grant_type(PolKitGrant* grant, PolKitResult type, void* d);
    static void conversation_done(PolKitGrant* grant, polkit_bool_t obtainedPrivilege, polkit_bool_t invalidData, void* d);
    static void polkit_config_changed(PolKitContext* context, void*);
};

#endif
