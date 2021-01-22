// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <optional>

#include <PolkitQt1/Agent/Session>

#include <QPointer>
#include <QQmlApplicationEngine>

#include "context.h"
#include "identitymodel.h"

class Listener;
class IdentityModel;

struct Context::Private {
    PolkitQt1::Agent::AsyncResult *result = nullptr;
    QString cookie;
    QPointer<PolkitQt1::Agent::Session> session;
    QJsonObject details;

    Listener *parent = nullptr;
    IdentityModel *identities = nullptr;

    bool canFingerprint = false;

    std::optional<int> customIdentity;
};