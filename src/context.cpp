// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "context.h"
#include "context_p.h"
#include "identitymodel.h"
#include "listener.h"

#include <PolkitQt1/Agent/Session>

Context::Context(ContextArgs args, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    qRegisterMetaType<IdentityModel *>();

    d->session = args.session;
    d->cookie = args.cookie;
    d->result = args.result;
    d->parent = qobject_cast<Listener *>(parent);
    d->identities = args.identityModel;
    d->details = args.details;

    Q_ASSERT(d->parent != nullptr);

    connectSession();
}

void Context::accept(const QString &password)
{
    d->session->setResponse(password);

    Q_EMIT accepted();
}

void Context::cancel()
{
    if (!d->session.isNull()) {
        d->session->cancel();
    }
    Q_EMIT complete();
}

IdentityModel *Context::identityModel() const
{
    return d->identities;
}

void Context::connectSession()
{
    connect(d->session.data(), &PolkitQt1::Agent::Session::request, this, [this](QString s, bool b) {
        Q_UNUSED(b);

        d->canFingerprint = s.toLower().contains("swipe finger");
        Q_EMIT canFingerprintChanged();
    });
    connect(d->session.data(), &PolkitQt1::Agent::Session::completed, this, [this](bool ok) {
        if (ok) {
            if (!d->session.isNull()) {
                d->session->result()->setCompleted();
            } else {
                d->result->setCompleted();
            }
            Q_EMIT complete();
        }
    });
    connect(d->session.data(), &PolkitQt1::Agent::Session::showError, this, [](QString err) {
        qDebug() << err;
    });

    d->session->initiate();
}

bool Context::canFingerprint() const
{
    return d->canFingerprint;
}

QString Context::currentUsername() const
{
    if (d->customIdentity.has_value()) {
        return d->identities->dataForUser(*d->customIdentity).first;
    }
    return d->identities->currentUserData().first;
}

QUrl Context::currentAvatar() const
{
    if (d->customIdentity.has_value()) {
        return QUrl::fromLocalFile(d->identities->dataForUser(*d->customIdentity).second);
    }
    return QUrl::fromLocalFile(d->identities->currentUserData().second);
}

QJsonObject Context::details() const
{
    return d->details;
}

void Context::useIdentity(int idx)
{
    if (d->customIdentity == idx) {
        return;
    }

    if (!d->session.isNull()) {
        d->session->deleteLater();

        d->session = new PolkitQt1::Agent::Session(d->identities->userIdentity(idx), d->cookie, d->result);
        connectSession();
    }

    d->customIdentity = idx;
    Q_EMIT identityChanged();
}
