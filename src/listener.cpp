// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include <PolkitQt1/Agent/Session>
#include <PolkitQt1/Authority>
#include <PolkitQt1/Details>

#include <QApplication>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSessionManager>

#include <KLocalizedContext>
#include <KLocalizedString>

#include "context.h"
#include "identitymodel.h"
#include "listener.h"
#include "listener_p.h"

extern void reexec();

Listener::Listener(QObject *parent)
    : PolkitQt1::Agent::Listener(parent)
    , d(new Private)
{
    d->identityModel = QSharedPointer<IdentityModel>(new IdentityModel);

    // We want to ensure that the session never closes while we're showing
    // a polkit dialog.
    auto blockSessionClose = [this](QSessionManager &sm) {
        if (!d->engine.isNull()) {
            sm.setRestartHint(QSessionManager::RestartNever);
        }
    };
    QObject::connect(qApp, &QApplication::commitDataRequest, blockSessionClose);
    QObject::connect(qApp, &QApplication::saveStateRequest, blockSessionClose);
}

void Listener::initiateAuthentication(const QString &actionID,
                                      const QString &message,
                                      const QString &iconName,
                                      const PolkitQt1::Details &details,
                                      const QString &cookie,
                                      const PolkitQt1::Identity::List &identities,
                                      PolkitQt1::Agent::AsyncResult *result)
{
    Q_UNUSED(iconName)
    Q_UNUSED(details)

    if (!d->engine.isNull()) {
        result->setError(i18n("An authentication dialog is already running."));
        result->setCompleted();
        return;
    }

    d->identityModel->setIdentities(identities);

    d->engine = QPointer<QQmlApplicationEngine>(new QQmlApplicationEngine);
    const QUrl url(
        qEnvironmentVariableIsSet("QT_QUICK_CONTROLS_MOBILE") ?
            QStringLiteral("qrc:/MobileDialog.qml") :
            QStringLiteral("qrc:/Dialog.qml"));
    connect(
        d->engine.data(),
        &QQmlApplicationEngine::objectCreated,
        this,
        [url](QObject *obj, const QUrl &objUrl) {
            if ((obj == nullptr) && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    auto [display, avatar] = d->identityModel->currentUserData();
    QJsonObject actionDetails;

    for (const auto &desc : PolkitQt1::Authority::instance()->enumerateActionsSync()) {
        if (actionID == desc.actionId()) {
            actionDetails["description"] = desc.description();
            actionDetails["message"] = desc.message();
            actionDetails["vendor"] = desc.vendorName();
            actionDetails["otherMessage"] = message;

            break;
        }
    }

    auto context = new Context(
        ContextArgs{
            .session = QPointer(new PolkitQt1::Agent::Session(d->identityModel->userIdentity(), cookie, result)),
            .details = actionDetails,
            .result = result,
            .cookie = cookie,
            .identityModel = d->identityModel.data(),
        },
        this);
    connect(context, &Context::complete, [=] {
        d->engine->deleteLater();
        reexec();
    });

    d->engine->rootContext()->setContextObject(new KLocalizedContext(d->engine.data()));
    d->engine->setObjectOwnership(context, QQmlEngine::JavaScriptOwnership);
    d->engine->rootContext()->setContextProperty("context", context);
    d->engine->load(url);

    QObject* rootObject = d->engine->rootObjects().first();
    QWindow *window = qobject_cast<QWindow *>(rootObject);
    QPixmap blank(1, 1);
    blank.fill(Qt::transparent);
    window->setIcon(QIcon(blank));
}

bool Listener::initiateAuthenticationFinish()
{
    return true;
}

void Listener::cancelAuthentication()
{
    d->engine->deleteLater();
    reexec();
}

void Listener::listen()
{
    auto session = PolkitQt1::UnixSessionSubject(QCoreApplication::applicationPid());

    if (!registerListener(session, "/org/kde/PolicyKit1/AuthenticationAgent")) {
        qFatal("Failed to register the agent; ensure that no other PolKit agents are running.");
    }
}
