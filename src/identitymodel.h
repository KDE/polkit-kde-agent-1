// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <PolkitQt1/Identity>

#include <QAbstractListModel>

class IdentityModel : public QAbstractListModel
{
    Q_OBJECT

    PolkitQt1::Identity::List _data;

public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::EditRole) const override;

    void setIdentities(const PolkitQt1::Identity::List &data);
    [[nodiscard]] QPair<QString, QString> currentUserData();
    [[nodiscard]] QPair<QString, QString> dataForUser(int idx);
    QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] PolkitQt1::Identity userIdentity(int idx = -1);
};
