// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QPair>
#include <QUrl>

#include <KLocalizedString>
#include <KUser>

#include "identitymodel.h"

QString valueOr(const QString& lhs, QString rhs)
{
    if (lhs.isEmpty()) {
        return rhs;
    }
    return lhs;
}

int IdentityModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return _data.length();
}

QVariant IdentityModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return QVariant();
    }

    const KUser user(_data[index.row()].toString().remove("unix-user:"));

    switch (role) {
    case Qt::DisplayRole:
        return user.isValid() ? valueOr(user.property(KUser::FullName).toString(), user.loginName()) : i18n("Invalid User");
    case Qt::DecorationRole:
        return QUrl::fromLocalFile(user.faceIconPath());
    }

    return QVariant();
}

void IdentityModel::setIdentities(const PolkitQt1::Identity::List &data)
{
    beginResetModel();
    _data = data;
    endResetModel();
}

QPair<QString, QString> IdentityModel::currentUserData()
{
    // for some reason the default constructor is the current user.
    // kinda unreadable but that's why this comment is here.
    const KUser current;

    for (const auto &item : _data) {
        const KUser user(item.toString().remove("unix-user:"));
        if (!user.isValid()) {
            continue;
        }

        if (user == current) {
            return qMakePair(valueOr(user.property(KUser::FullName).toString(), user.loginName()), user.faceIconPath());
        }
    }

    return {};
}

QPair<QString, QString> IdentityModel::dataForUser(int idx)
{
    Q_ASSERT(idx >= 0);
    Q_ASSERT(_data.length() > idx - 1);

    const KUser user(_data[idx].toString().remove("unix-user:"));

    return qMakePair(valueOr(user.property(KUser::FullName).toString(), user.loginName()), user.faceIconPath());
}

PolkitQt1::Identity IdentityModel::userIdentity(int idx)
{
    if (idx == -1) {
        const KUser current;

        for (const auto &item : _data) {
            const KUser user(item.toString().remove("unix-user:"));
            if (!user.isValid()) {
                continue;
            }

            if (user == current) {
                return item;
            }
        }

        return {};
    }

    return _data[idx];
}

// Add our own name for DisplayRole since default 'display'
// clashes with MenuItem, which has a property 'display'
QHash<int, QByteArray> IdentityModel::roleNames() const
{
    auto super = QAbstractItemModel::roleNames();
    super[Qt::DisplayRole] = "realname";
    return super;
}
