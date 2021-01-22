// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <PolkitQt1/Agent/Session>

#include <QJSEngine>
#include <QJSValue>
#include <QJsonObject>
#include <QObject>
#include <QPointer>

class IdentityModel;

struct ContextArgs {
    QPointer<PolkitQt1::Agent::Session> session;
    QJsonObject details;

    PolkitQt1::Agent::AsyncResult *result;
    QString cookie;

    IdentityModel *identityModel;
};

class Context : public QObject
{
    Q_OBJECT

    Q_PROPERTY(IdentityModel *identityModel READ identityModel CONSTANT)
    Q_PROPERTY(QString currentUsername READ currentUsername NOTIFY identityChanged)
    Q_PROPERTY(QUrl currentAvatar READ currentAvatar NOTIFY identityChanged)
    Q_PROPERTY(QJsonObject details READ details CONSTANT)
    Q_PROPERTY(bool canFingerprint READ canFingerprint NOTIFY canFingerprintChanged)

public:
    Context(ContextArgs args, QObject *parent = nullptr);

    Q_INVOKABLE void accept(const QString &password);
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void useIdentity(int idx);

    IdentityModel *identityModel() const;
    QString currentUsername() const;
    QUrl currentAvatar() const;
    QJsonObject details() const;
    bool canFingerprint() const;

Q_SIGNALS:
    void accepted();
    void complete();
    void identityChanged();
    void canFingerprintChanged();

private:
    struct Private;
    QSharedPointer<Private> d;

    void connectSession();
};
