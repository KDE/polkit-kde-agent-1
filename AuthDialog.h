/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2007-2008 Gökçen Eraslan <gokcen@pardus.org.tr>
    SPDX-FileCopyrightText: 2008 Daniel Nicoletti <dantti85-pk@yahoo.com.br>
    SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>

#include <PolkitQt1/ActionDescription>
#include <PolkitQt1/Identity>

#include "ui_AuthDialog.h"
#include "ui_authdetails.h"

namespace PolkitQt1
{
class Details;
}

class AuthDialog : public QDialog, private Ui::AuthDialog
{
    Q_OBJECT
public:
    AuthDialog(const QString &actionId,
               const QString &message,
               const QString &iconName,
               const PolkitQt1::Details &details,
               const PolkitQt1::Identity::List &identities,
               WId parent);
    ~AuthDialog() override;

    void setRequest(const QString &request, bool requiresAdmin);
    void setOptions();
    QString password() const;
    void authenticationFailure();

    PolkitQt1::Identity adminUserSelected() const;

    PolkitQt1::ActionDescription m_actionDescription;

Q_SIGNALS:
    void adminUserSelected(PolkitQt1::Identity);
    void okClicked();

public Q_SLOTS:
    void accept() override;

private Q_SLOTS:
    void checkSelectedUser();

private:
    QString m_message;

    void createUserCB(const PolkitQt1::Identity::List &identities);
};

class AuthDetails : public QWidget, private Ui::AuthDetails
{
    Q_OBJECT
public:
    AuthDetails(const PolkitQt1::Details &details, const PolkitQt1::ActionDescription &actionDescription, QWidget *parent);

private Q_SLOTS:
    void openUrl(const QString &);
};

#endif // AUTHDIALOG_H
