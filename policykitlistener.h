#ifndef POLICYKITLISTENER_H
#define POLICYKITLISTENER_H

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

#include <PolkitQt1/Agent/Listener>

#include <QtCore/QWeakPointer>
#include <QtCore/QHash>

class AuthDialog;

using namespace PolkitQt1::Agent;

class PolicyKitListener : public Listener
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Polkit1AuthAgent")
public:
    PolicyKitListener(QObject *parent = 0);
    virtual ~PolicyKitListener();

public slots:
    void initiateAuthentication(const QString &actionId,
                                const QString &message,
                                const QString &iconName,
                                const PolkitQt1::Details &details,
                                const QString &cookie,
                                const PolkitQt1::Identity::List &identities,
                                PolkitQt1::Agent::AsyncResult* result);
    bool initiateAuthenticationFinish();
    void cancelAuthentication();

    void tryAgain();
    void finishObtainPrivilege();

    void request(const QString &request, bool echo);
    void completed(bool gainedAuthorization);
    void showError(const QString &text);

    void handshakeForAction(const QString &action, qulonglong wID);
    /*    void showInfo(const QString &text);    */
private:
    QWeakPointer<AuthDialog> m_dialog;
    QWeakPointer<Session> m_session;
    bool m_inProgress;
    bool m_gainedAuthorization;
    bool m_wasCancelled;
    int m_numTries;
    PolkitQt1::Identity::List m_identities;
    PolkitQt1::Agent::AsyncResult* m_result;
    QString m_cookie;
    PolkitQt1::Identity m_selectedUser;
    QHash< QString, qulonglong > m_actionsToWID;

private slots:
    void dialogAccepted();
    void dialogCanceled();
    void userSelected(const PolkitQt1::Identity &identity);
};

#endif
