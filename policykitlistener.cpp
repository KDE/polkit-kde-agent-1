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

#include <QDBusConnection>
#include <QDebug>

#include <KWindowSystem>

#include <PolkitQt1/Agent/Listener>
#include <PolkitQt1/Agent/Session>
#include <PolkitQt1/Subject>
#include <PolkitQt1/Identity>
#include <PolkitQt1/Details>

#include "policykitlistener.h"
#include "AuthDialog.h"
#include "polkit1authagentadaptor.h"

PolicyKitListener::PolicyKitListener(QObject *parent)
        : Listener(parent)
        , m_inProgress(false)
        , m_selectedUser(0)
{
    (void) new Polkit1AuthAgentAdaptor(this);

    if (!QDBusConnection::sessionBus().registerObject("/org/kde/Polkit1AuthAgent", this,
                                                     QDBusConnection::ExportScriptableSlots |
                                                     QDBusConnection::ExportScriptableProperties |
                                                     QDBusConnection::ExportAdaptors)) {
        qWarning() << "Could not initiate DBus helper!";
    }

    qDebug() << "Listener online";
}

PolicyKitListener::~PolicyKitListener()
{
}

void PolicyKitListener::setWIdForAction(const QString& action, qulonglong wID)
{
    qDebug() << "On to the handshake";
    m_actionsToWID[action] = wID;
}

void PolicyKitListener::initiateAuthentication(const QString &actionId,
        const QString &message,
        const QString &iconName,
        const PolkitQt1::Details &details,
        const QString &cookie,
        const PolkitQt1::Identity::List &identities,
        PolkitQt1::Agent::AsyncResult* result)
{
    qDebug() << "Initiating authentication";

    if (m_inProgress) {
        result->setError(i18n("Another client is already authenticating, please try again later."));
        result->setCompleted();
        qDebug() << "Another client is already authenticating, please try again later.";
        return;
    }

    m_identities = identities;
    m_cookie = cookie;
    m_result = result;
    m_session.clear();

    m_inProgress = true;

    WId parentId = 0;

    if (m_actionsToWID.contains(actionId)) {
        parentId = m_actionsToWID[actionId];
    }

    m_dialog = new AuthDialog(actionId, message, iconName, details, identities, parentId);
    connect(m_dialog.data(), SIGNAL(okClicked()), SLOT(dialogAccepted()));
    connect(m_dialog.data(), SIGNAL(cancelClicked()), SLOT(dialogCanceled()));
    connect(m_dialog.data(), SIGNAL(adminUserSelected(PolkitQt1::Identity)), SLOT(userSelected(PolkitQt1::Identity)));

    qDebug() << "WinId of the dialog is " << m_dialog.data()->winId() << m_dialog.data()->effectiveWinId();
    m_dialog.data()->setOptions();
    m_dialog.data()->show();
    KWindowSystem::forceActiveWindow(m_dialog.data()->winId());
    qDebug() << "WinId of the shown dialog is " << m_dialog.data()->winId() << m_dialog.data()->effectiveWinId();

    if (identities.length() == 1) {
        m_selectedUser = identities[0];
    } else {
        m_selectedUser = m_dialog.data()->adminUserSelected();
    }

    m_numTries = 0;
    tryAgain();
}

void PolicyKitListener::tryAgain()
{
    qDebug() << "Trying again";
    m_wasCancelled = false;

    // We will create new session only when some user is selected
    if (m_selectedUser.isValid()) {
        m_session = new Session(m_selectedUser, m_cookie, m_result);
        connect(m_session.data(), SIGNAL(request(QString,bool)), this, SLOT(request(QString,bool)));
        connect(m_session.data(), SIGNAL(completed(bool)), this, SLOT(completed(bool)));
        connect(m_session.data(), SIGNAL(showError(QString)), this, SLOT(showError(QString)));

        m_session.data()->initiate();
    }

}

void PolicyKitListener::finishObtainPrivilege()
{
    qDebug() << "Finishing obtaining privileges";

    // Number of tries increase only when some user is selected
    if (m_selectedUser.isValid()) {
        m_numTries++;
    }

    if (!m_gainedAuthorization && !m_wasCancelled && !m_dialog.isNull()) {
        m_dialog.data()->authenticationFailure();

        if (m_numTries < 3) {
            m_session.data()->deleteLater();

            tryAgain();
            return;
        }
    }

    if (!m_session.isNull()) {
        m_session.data()->result()->setCompleted();
    } else {
        m_result->setCompleted();
    }
    m_session.data()->deleteLater();

    if (!m_dialog.isNull()) {
        m_dialog.data()->hide();
        m_dialog.data()->deleteLater();
    }

    m_inProgress = false;

    qDebug() << "Finish obtain authorization:" << m_gainedAuthorization;
}

bool PolicyKitListener::initiateAuthenticationFinish()
{
    qDebug() << "Finishing authentication";
    return true;
}

void PolicyKitListener::cancelAuthentication()
{
    qDebug() << "Cancelling authentication";

    m_wasCancelled = true;
    finishObtainPrivilege();
}

void PolicyKitListener::request(const QString &request, bool echo)
{
    Q_UNUSED(echo);
    qDebug() << "Request: " << request;

    if (!m_dialog.isNull()) {
        m_dialog.data()->setRequest(request, m_selectedUser.isValid() &&
                m_selectedUser.toString() == "unix-user:root");
    }
}

void PolicyKitListener::completed(bool gainedAuthorization)
{
    qDebug() << "Completed: " << gainedAuthorization;

    m_gainedAuthorization = gainedAuthorization;

    finishObtainPrivilege();
}

void PolicyKitListener::showError(const QString &text)
{
    qDebug() << "Error: " << text;
}

void PolicyKitListener::dialogAccepted()
{
    qDebug() << "Dialog accepted";

    if (!m_dialog.isNull()) {
        m_session.data()->setResponse(m_dialog.data()->password());
    }
}

void PolicyKitListener::dialogCanceled()
{
    qDebug() << "Dialog cancelled";

    m_wasCancelled = true;
    if (!m_session.isNull()) {
        m_session.data()->cancel();
    }

    finishObtainPrivilege();
}

void PolicyKitListener::userSelected(const PolkitQt1::Identity &identity)
{
    m_selectedUser = identity;
    // If some user is selected we must destroy existing session
    if (!m_session.isNull()) {
        m_session.data()->deleteLater();
    }
    tryAgain();
}
