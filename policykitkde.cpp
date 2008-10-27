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

#include "policykitkde.h"
#include "authenticationagentadaptor.h"

#include <kapplication.h>
#include <kdebug.h>
#include <qstring.h>
#include <kwindowsystem.h>

#include "qdbusconnection.h"

#include <qvariant.h>
#include <qsocketnotifier.h>

//policykit header
#include <polkit-dbus/polkit-dbus.h>

#include "authdialog.h"

PolicyKitKDE* PolicyKitKDE::m_self;

//----------------------------------------------------------------------------

PolicyKitKDE::PolicyKitKDE(QObject* parent)
    : QObject(parent)
{
    Q_ASSERT(!m_self);
    m_self = this;

    (void) new AuthenticationAgentAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService("org.freedesktop.PolicyKit.AuthenticationAgent"))
        kError() << "anothe authentication agent already running";

    if (!QDBusConnection::sessionBus().registerObject("/", this)) {
        kError() << "unable to register service interface to dbus";
    }

    m_context = polkit_context_new();
    if (m_context == NULL)
    {
        kDebug() << "Could not get a new PolKitContext.";
        return;
    }

    polkit_context_set_load_descriptions(m_context);

    polkit_context_set_config_changed( m_context, polkit_config_changed, NULL );
    polkit_context_set_io_watch_functions (m_context, polkit_add_watch, polkit_remove_watch);

    PolKitError* error = NULL;
    if (!polkit_context_init (m_context, &error))
    {
        QString msg("Could not initialize PolKitContext");
        if (polkit_error_is_set(error))
        {
            kError() << msg <<  ": " << polkit_error_get_error_message(error);
            polkit_error_free( error );
        }
        else
            kError() << msg;
    }
    //TODO: add kill_timer
}

//----------------------------------------------------------------------------

PolicyKitKDE::~PolicyKitKDE()
{
    m_self = 0L;
}


//----------------------------------------------------------------------------

void PolicyKitKDE::watchActivated(int fd)
{
    Q_ASSERT(m_watches.contains(fd));

    kDebug() << "watchActivated" << fd;

    polkit_context_io_func (m_context, fd);
}

//----------------------------------------------------------------------------

int PolicyKitKDE::polkit_add_watch(PolKitContext *context, int fd)
{
    kDebug() << "polkit_add_watch" << context << fd;

    QSocketNotifier *notify = new QSocketNotifier(fd, QSocketNotifier::Read, m_self);
    m_self->m_watches[fd] = notify;

    notify->connect(notify, SIGNAL(activated(int)), m_self, SLOT(watchActivated(int)));

    return fd; // use simply the fd as the unique id for the watch
}


//----------------------------------------------------------------------------

void PolicyKitKDE::polkit_remove_watch(PolKitContext *context, int fd)
{
    kDebug() << "polkit_remove_watch" << context << fd;
    Q_ASSERT(m_self->m_watches.contains(fd));

    QSocketNotifier* notify = m_self->m_watches.take(fd);
    delete notify;
}

//----------------------------------------------------------------------------

void PolicyKitKDE::polkit_config_changed( PolKitContext* context, void* )
{
    kDebug() << "polkit_config_changed" << context;
    // Nothing to do here it seems (?).
}

//----------------------------------------------------------------------------

bool PolicyKitKDE::ObtainAuthorization(const QString& actionId, uint wid, uint pid)
{
    kDebug() << "Start obtain authorization:" << actionId << wid << pid;

    PolKitError *error = NULL;

    PolKitAction *action = polkit_action_new();
    if (action == NULL)
    {
        kError() << "Could not create new polkit action.";
        return false;
    }

    polkit_bool_t setActionResult = polkit_action_set_action_id(action, actionId.toLatin1());
    if (!setActionResult)
    {
        kError() << "Could not set actionid.";
        return false;
    }

    kError() << "Getting policy cache...";
    PolKitPolicyCache *cache = polkit_context_get_policy_cache(m_context);
    if (cache == NULL)
    {
        kWarning() << "Could not get policy cache.";
    //    return false;
    }

    kDebug() << "Getting policy cache entry for an action...";
    PolKitPolicyFileEntry *entry = polkit_policy_cache_get_entry(cache, action);
    if (entry == NULL)
    {
        kWarning() << "Could not get policy entry for action.";
    //    return false;
    }

    kDebug() << "Getting action message...";
    QString message = QString::fromLocal8Bit(polkit_policy_file_entry_get_action_message(entry));
    if (message.isEmpty())
    {
        kWarning() << "Could not get action message for action.";
    //    return false;
    }
    else
    {
        kDebug() << "Message of action: " << message;
    }

    DBusError dbuserror;
    dbus_error_init (&dbuserror);
    DBusConnection *bus = dbus_bus_get (DBUS_BUS_SYSTEM, &dbuserror);
    if (bus == NULL) 
    {
        kError() << "Could not connect to system bus.";
        return false;
    }

    PolKitCaller *caller = polkit_caller_new_from_pid(bus, pid, &dbuserror);
    if (caller == NULL)
    {
        kError() << QString("Could not define caller from pid: %1")
            .arg(QDBusError((const DBusError *)&dbuserror).message());
        return false;
    }

    PolKitResult polkitresult;

    polkitresult = polkit_context_is_caller_authorized(m_context, action, caller, false, &error);
    if (polkit_error_is_set (error))
    {
        kError() << "Could not determine if caller is authorized for this action.";
        return false;
    }

    //TODO: Determine AdminAuthType, user, group...

    AuthDialog dia(message, polkitresult);
    if( wid != 0 )
        KWindowSystem::setMainWindow( &dia, wid );
    else
    {
        kapp->updateUserTimestamp(); // make it get focus unconditionally :-/
    }
    dia.exec();

    // check again if user is authorized
    polkitresult = polkit_context_is_caller_authorized(m_context, action, caller, false, &error);
    if (polkit_error_is_set (error))
    {
        kError() << "Could not determine if caller is authorized for this action.";
        return false;
    }

    return false;
}
