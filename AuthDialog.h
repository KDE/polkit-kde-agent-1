/*  This file is part of the KDE project
    Copyright (C) 2007-2008 Gökçen Eraslan <gokcen@pardus.org.tr>
    Copyright (C) 2008 Daniel Nicoletti <dantti85-pk@yahoo.com.br>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#include <polkit/polkit.h>

#include <QtCore/QObject>

#include "ui_AuthDialog.h"
#include "ui_authdetails.h"

#include "policykitkde.h"

class AuthDialog : public KDialog, private Ui::AuthDialog
{
    Q_OBJECT
public:
    AuthDialog(PolKitPolicyFileEntry *entry, uint pid);
    ~AuthDialog();

    void setRequest(QString request, bool requiresAdmin);
    void setPasswordShowChars(bool showChars);
    void setOptions(KeepPassword keep, bool requiresAdmin, QStringList adminUsers);
    QString password() const;
    void incorrectPassword();
    KeepPassword keepPassword() const;


    QString adminUserSelected() const;
    QString selectCurrentAdminUser();

    QString m_appname;

signals:
    void adminUserSelected(QString adminUser);

public slots:
    virtual void accept();

private slots:
    void on_userCB_currentIndexChanged(int index);

private:
    void createUserCB(QStringList adminUsers);
};

class AuthDetails : public QWidget, private Ui::AuthDetails
{
    Q_OBJECT
public:
    AuthDetails(PolKitPolicyFileEntry *entry, QString appname, QWidget *parent);

private slots:
    void openUrl(const QString&);
    void openAction(const QString&);
};

#endif // AUTHDIALOG_H
