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

#include "policykitlistener.h"
#include "AuthDialog.h"

#include <KDebug>

#include <PolkitQt1/Agent/Listener>
#include <PolkitQt1/Agent/Session>
#include <PolkitQt1/Subject>
#include <PolkitQt1/Identity>
#include <PolkitQt1/Details>

PolicyKitListener::PolicyKitListener(QObject *parent)
        : Listener(parent)
        , m_inProgress(false)
        , m_selectedUser(0)
{
}

PolicyKitListener::~PolicyKitListener()
{
}

void PolicyKitListener::initiateAuthentication(const QString &actionId,
        const QString &message,
        const QString &iconName,
        PolkitQt1::Details *details,
        const QString &cookie,
        QList<PolkitQt1::Identity *> identities,
        PolkitQt1::Agent::AsyncResult* result)
{
    kDebug() << "Initiating authentication";

    if (m_inProgress) {
        result->setError(i18n("Another client is already authenticating, please try again later."));
        result->setCompleted();
        kDebug() << "Another client is already authenticating, please try again later.";
        return;
    }

    m_identities = identities;
    m_cookie = cookie;
    m_result = result;
    m_session = 0;
    if (identities.length() == 1) {
        m_selectedUser = identities[0];
    } else {
        m_selectedUser = 0;
    }

    m_inProgress = true;

    m_dialog = new AuthDialog(actionId, message, iconName, details, identities);
    connect(m_dialog, SIGNAL(okClicked()), SLOT(dialogAccepted()));
    connect(m_dialog, SIGNAL(cancelClicked()), SLOT(dialogCanceled()));
    connect(m_dialog, SIGNAL(adminUserSelected(PolkitQt1::Identity *)), SLOT(userSelected(PolkitQt1::Identity *)));

    m_dialog->setOptions();
    m_dialog->show();

    m_numTries = 0;
    tryAgain();
}

void PolicyKitListener::tryAgain()
{
    kDebug() << "Trying again";
//  test!!!
    m_wasCancelled = false;

    // We will create new session only when some user is selected
    if (m_selectedUser != 0) {
        m_session = new Session(m_selectedUser, m_cookie, m_result);
        connect(m_session, SIGNAL(request(QString, bool)), this, SLOT(request(QString, bool)));
        connect(m_session, SIGNAL(completed(bool)), this, SLOT(completed(bool)));
        connect(m_session, SIGNAL(showError(QString)), this, SLOT(showError(QString)));

        m_session->initiate();
    }

}

void PolicyKitListener::finishObtainPrivilege()
{
    kDebug() << "Finishing obtaining privileges";

    // Number of tries increase only when some user is selected
    if (m_selectedUser != 0) {
        m_numTries++;
    }

    if (!m_gainedAuthorization && !m_wasCancelled && !m_dialog.isNull()) {
        m_dialog->authenticationFailure();

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
    m_session->deleteLater();

    if (m_dialog) {
        m_dialog->deleteLater();
        m_dialog = 0;
    }

    m_inProgress = false;

    kDebug() << "Finish obtain authorization:" << m_gainedAuthorization;
}

bool PolicyKitListener::initiateAuthenticationFinish()
{
    kDebug() << "Finishing authentication";
    return true;
}

void PolicyKitListener::cancelAuthentication()
{
    kDebug() << "Cancelling authentication";

    m_wasCancelled = true;
    finishObtainPrivilege();
}

void PolicyKitListener::request(const QString &request, bool echo)
{
    kDebug() << "Request: " << request;

    if (m_dialog) {
        m_dialog->setRequest(request, echo);
    }
}

void PolicyKitListener::completed(bool gainedAuthorization)
{
    kDebug() << "Completed: " << gainedAuthorization;

    m_gainedAuthorization = gainedAuthorization;

    finishObtainPrivilege();
}

void PolicyKitListener::showError(const QString &text)
{
    kDebug() << "Error: " << text;
}

void PolicyKitListener::dialogAccepted()
{
    kDebug() << "Dialog accepted";

    if (m_dialog)
        m_session->setResponse(m_dialog->password());
}

void PolicyKitListener::dialogCanceled()
{
    kDebug() << "Dialog cancelled";

    m_wasCancelled = true;
    if (!m_session.isNull()) {
        m_session->cancel();
    }

    finishObtainPrivilege();
}

void PolicyKitListener::userSelected(PolkitQt1::Identity *identity)
{
    m_selectedUser = identity;
    // If some user is selected we must destroy existing session
    if (!m_session.isNull()) {
        m_session.data()->deleteLater();
    }
    tryAgain();
}
