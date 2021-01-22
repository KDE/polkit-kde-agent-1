// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <PolkitQt1/Agent/Listener>

#include <QSharedPointer>

class Listener : public PolkitQt1::Agent::Listener
{
    Q_OBJECT

public:
    explicit Listener(QObject *parent = nullptr);

    struct Private;
    QSharedPointer<Private> d;

    void listen();

public Q_SLOTS:
    void initiateAuthentication(const QString &actionID,
                                const QString &message,
                                const QString &iconName,
                                const PolkitQt1::Details &details,
                                const QString &cookie,
                                const PolkitQt1::Identity::List &identities,
                                PolkitQt1::Agent::AsyncResult *result) override;

    bool initiateAuthenticationFinish() override;
    void cancelAuthentication() override;
};