/*  This file is part of the KDE project
    Copyright (C) 2007-2008 Gökçen Eraslan <gokcen@pardus.org.tr>
    Copyright (C) 2008 Dirk Mueller <mueller@kde.org>
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

#include "authdialog.h"

#include <QProcess>

#include <KDebug>
#include <KToolInvocation>

#include "ui_authdetails.h"

/*
 *  Constructs a AuthDialog which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AuthDialog::AuthDialog(PolKitPolicyFileEntry *entry, uint pid)
        : KDialog(0)
{
    setupUi(mainWidget());

    setButtons(Ok | Cancel | Details);

    kDebug() << "Getting action message...";
    QString actionMessage = QString::fromLocal8Bit(polkit_policy_file_entry_get_action_message(entry));
    if (actionMessage.isEmpty()) {
        kWarning() << "Could not get action message for action.";
        lblHeader->hide();
    } else {
        kDebug() << "Message of action: " << actionMessage;
        lblHeader->setText("<h3>" + actionMessage + "</h3>");
        setCaption(actionMessage);
    }

    QPixmap icon = KIconLoader::global()->loadIcon(polkit_policy_file_entry_get_action_icon_name(entry),
                   KIconLoader::NoGroup, KIconLoader::SizeHuge, KIconLoader::DefaultState, QStringList(), NULL, true);
    if (icon.isNull())
        icon = KIconLoader::global()->loadIcon("dialog-password",
                                               KIconLoader::NoGroup, KIconLoader::SizeHuge);
    setWindowIcon(icon);
    lblPixmap->setPixmap(icon);

    cbUsers->hide();
    lePassword->setFocus();

    AuthDetails* details = new AuthDetails(this);

    QString appname;
    char tmp[ PATH_MAX ];
    if (polkit_sysdeps_get_exe_for_pid_with_helper(pid, tmp, sizeof(tmp) - 1) < 0)
        appname = i18n("Unknown");
    else
        appname = QString::fromLocal8Bit(tmp);

    details->app_label->setText(appname);

    QString actionId = polkit_policy_file_entry_get_id(entry);
    details->action_label->setText(actionId);
    details->action_label->setUrl(actionId);

    details->vendor_label->setText(polkit_policy_file_entry_get_action_vendor(entry));
    details->vendor_label->setUrl(polkit_policy_file_entry_get_action_vendor_url(entry));
    setDetailsWidget(details);

//     resize( sizeHint() + QSize( 100, 100 )); // HACK broken QLabel layouting

    errorMessageKTW->hide();
}

AuthDialog::~AuthDialog()
{
}

void AuthDialog::accept()
{
    // Do nothing, do not close the dialog. This is needed so that the dialog stays
    return;
}

// void AuthDialog::setHeader(const QString &header)
// {
//     lblHeader->setText("<h3>" + header + "</h3>");
// }

void AuthDialog::setContent(const QString &msg)
{
    lblContent->setText(msg);
}

void AuthDialog::setPasswordPrompt(const QString& prompt)
{
    lblPassword->setText(prompt);
}

QString AuthDialog::password() const
{
    return lePassword->text();
}

void AuthDialog::incorrectPassword()
{
    lePassword->clear();
    errorMessageKTW->setText(i18n("Incorrect password, please try again."), KTitleWidget::ErrorMessage);
    QFont bold = font();
    bold.setBold(true);
    lblPassword->setFont(bold);
    lePassword->clear();
    lePassword->setFocus();
}

void AuthDialog::showKeepPassword(KeepPassword keep)
{
    switch (keep) {
    case KeepPasswordNo:
        cbRemember->hide();
        cbSessionOnly->hide();
        break;
    case KeepPasswordSession:
        cbRemember->setText(i18n("Remember authorization for this session"));
        cbRemember->show();
        cbSessionOnly->hide();
        break;
    case KeepPasswordAlways:
        cbRemember->setText(i18n("Remember authorization"));
        cbRemember->show();
        cbSessionOnly->show();
        break;
    }
}

KeepPassword AuthDialog::keepPassword() const
{
    if (cbRemember->isHidden()) // cannot make it keep
        return KeepPasswordNo;
    if (cbSessionOnly->isHidden()) // can keep only for session
        return cbRemember->isChecked() ? KeepPasswordSession : KeepPasswordNo;
    // can keep either way
    if (cbRemember->isChecked())
        return cbSessionOnly->isChecked() ? KeepPasswordSession : KeepPasswordAlways;
    return KeepPasswordNo;
}

AuthDetails::AuthDetails(QWidget* parent)
        : QWidget(parent)
{
    setupUi(this);
    connect(vendor_label, SIGNAL(leftClickedUrl(const QString&)), SLOT(openUrl(const QString&)));
    connect(action_label, SIGNAL(leftClickedUrl(const QString&)), SLOT(openAction(const QString&)));
}

void AuthDetails::openUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

void AuthDetails::openAction(const QString &url)
{
    QProcess::startDetached("polkit-kde-authorization", QStringList() << url);
}
