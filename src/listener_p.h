// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include <PolkitQt1/Agent/Session>

#include <QQmlApplicationEngine>

#include "identitymodel.h"
#include "listener.h"

class IdentityModel;

struct Listener::Private {
    QSharedPointer<IdentityModel> identityModel;
    QPointer<QQmlApplicationEngine> engine;
};
