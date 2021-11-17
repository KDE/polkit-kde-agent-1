/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "QuickAuthDialog.h"
#include "IdentitiesModel.h"
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KUser>
#include <PolkitQt1/Authority>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QTimer>

QuickAuthDialog::QuickAuthDialog(const QString &actionId,
                                 const QString &message,
                                 const QString &iconName,
                                 const PolkitQt1::Details &details,
                                 const PolkitQt1::Identity::List &identities,
                                 WId parent)
    : QObject(nullptr)
{
    auto engine = new QQmlApplicationEngine(this);
    QVariantMap props = {
        {"iconName", iconName},
        {"mainText", message},
    };
    const auto actions = PolkitQt1::Authority::instance()->enumerateActionsSync();
    for (const PolkitQt1::ActionDescription &desc : actions) {
        if (actionId == desc.actionId()) {
            qDebug() << "Action description has been found";
            props.insert("description", QVariant::fromValue(desc));
            break;
        }
    }
    engine->setInitialProperties(props);
    engine->rootContext()->setContextObject(new KLocalizedContext(engine));
    engine->load("qrc:/QuickAuthDialog.qml");
    m_theDialog = qobject_cast<QQuickWindow *>(engine->rootObjects().constFirst());

    auto idents = qobject_cast<IdentitiesModel *>(m_theDialog->property("identitiesModel").value<QObject *>());
    idents->setIdentities(identities, false);
    if (!identities.isEmpty()) {
        m_theDialog->setProperty("identitiesCurrentIndex", idents->indexForUser(KUser().loginName()));
    }

    connect(m_theDialog, SIGNAL(accept()), this, SIGNAL(okClicked()));
    connect(m_theDialog, SIGNAL(reject()), this, SIGNAL(rejected()));
}

enum KirigamiInlineMessageTypes { Information = 0, Positive = 1, Warning = 2, Error = 3 };

QString QuickAuthDialog::password() const
{
    return m_theDialog->property("password").toString();
}

void QuickAuthDialog::showError(const QString &message)
{
    m_theDialog->setProperty("inlineMessageType", Error);
    m_theDialog->setProperty("inlineMessageText", message);
}

void QuickAuthDialog::showInfo(const QString &message)
{
    m_theDialog->setProperty("inlineMessageType", Information);
    m_theDialog->setProperty("inlineMessageText", message);
}

PolkitQt1::Identity QuickAuthDialog::adminUserSelected() const
{
    return PolkitQt1::Identity::fromString(m_theDialog->property("selectedIdentity").toString());
}

void QuickAuthDialog::authenticationFailure()
{
    QTimer::singleShot(0, m_theDialog, SLOT(authenticationFailure()));
}

void QuickAuthDialog::setRequest(const QString &request, bool requiresAdmin)
{
    PolkitQt1::Identity identity = adminUserSelected();
    if (request.startsWith(QLatin1String("password:"), Qt::CaseInsensitive)) {
        if (requiresAdmin) {
            if (!identity.isValid()) {
                m_theDialog->setProperty("subtitle", i18n("Password for root:"));
            } else {
                m_theDialog->setProperty("subtitle", i18n("Password for %1:", identity.toString().remove("unix-user:")));
            }
        } else {
            m_theDialog->setProperty("subtitle", i18n("Password:"));
        }
    } else if (request.startsWith(QLatin1String("password or swipe finger:"), Qt::CaseInsensitive)) {
        if (requiresAdmin) {
            if (!identity.isValid()) {
                m_theDialog->setProperty("subtitle", i18n("Password or swipe finger for root:"));
            } else {
                m_theDialog->setProperty("subtitle", i18n("Password or swipe finger for %1:", identity.toString().remove("unix-user:")));
            }
        } else {
            m_theDialog->setProperty("subtitle", i18n("Password or swipe finger:"));
        }
    } else {
        m_theDialog->setProperty("subtitle", request);
    }
}

void QuickAuthDialog::show()
{
    QTimer::singleShot(0, m_theDialog, SLOT(show()));
}

void QuickAuthDialog::hide()
{
    QTimer::singleShot(0, m_theDialog, SLOT(hide()));
}
