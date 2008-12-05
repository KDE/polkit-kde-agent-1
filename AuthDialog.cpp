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

#include "AuthDialog.h"

#include <QProcess>

#include <KDebug>

#include <KToolInvocation>
#include <KUser>

/*
 *  Constructs a AuthDialog which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AuthDialog::AuthDialog(PolKitPolicyFileEntry *entry, uint pid)
        : KDialog(0, Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint)
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

    char tmp[ PATH_MAX ];
    if (polkit_sysdeps_get_exe_for_pid_with_helper(pid, tmp, sizeof(tmp) - 1) < 0) {
        m_appname.clear();
        AuthDetails *details = new AuthDetails(entry, i18n("Unknown"), this);
        setDetailsWidget(details);
    }
    else {
        m_appname = QString::fromLocal8Bit(tmp);
        AuthDetails *details = new AuthDetails(entry, m_appname, this);
        setDetailsWidget(details);
    }

    userCB->hide();
    lePassword->setFocus();

    errorMessageKTW->hide();
    connect(userCB, SIGNAL(currentIndexChanged(int)), this, SLOT(on_userCB_currentIndexChanged(int)));
}

AuthDialog::~AuthDialog()
{
}

void AuthDialog::accept()
{
    // Do nothing, do not close the dialog. This is needed so that the dialog stays
    return;
}

void AuthDialog::setRequest(QString request, bool requireAdmin)
{
    if (request.left(9).compare("password:", Qt::CaseInsensitive) == 0) {
        if (requireAdmin) {
            if (!userCB->itemData(userCB->currentIndex()).isNull()) {
                lblPassword->setText(i18n("Password for %1:",
                      KUser::KUser(userCB->itemData(userCB->currentIndex()).toInt()).loginName()));
            } else {
                    lblPassword->setText(i18n("Password for root:"));
            }
        } else {
                lblPassword->setText(i18n("Password:"));
        }
    } else if (request.left(25).compare("password or swipe finger:", Qt::CaseInsensitive) == 0) {
        if (requireAdmin) {
            if (!userCB->itemData(userCB->currentIndex()).isNull()) {
                lblPassword->setText(i18n("Password or swipe finger for %1:",
                      KUser::KUser(userCB->itemData(userCB->currentIndex()).toInt()).loginName()));
            } else {
                lblPassword->setText(i18n("Password or swipe finger for root:"));
            }
        } else {
            lblPassword->setText(i18n("Password or swipe finger:"));
        }
    } else {
        lblPassword->setText(request);
    }

    if (requireAdmin) {
        if (m_appname.isEmpty())
            lblContent->setText(i18n("An application is attempting to perform an action that requires privileges."
                    " Authentication as the super user is required to perform this action."));
        else
            lblContent->setText(i18n("The application %1 is attempting to perform an action that requires privileges."
                    " Authentication as the super user is required to perform this action.", m_appname));

    } else {
        if (m_appname.isEmpty())
            lblContent->setText(i18n("An application is attempting to perform an action that requires privileges."
                    " Authentication is required to perform this action."));
        else
            lblContent->setText(i18n("The application %1 is attempting to perform an action that requires privileges."
                    " Authentication is required to perform this action.", m_appname));

    }
}

void AuthDialog::createUserCB(char **admin_users)
{
        /* if we've already built the list of admin users once, then avoid
         * doing it again.. (this is mainly used when the user entered the
         * wrong password and the dialog is recycled)
         */
        if (userCB->count())
                return;

        // Adds a Dummy user
        userCB->addItem(i18n("Select user"));

        // For each user
        for (int i = 0; admin_users[i] != NULL; i++) {
            // First check to see if the user is valid
            KUser user = KUser::KUser(admin_users[i]);
            if (!user.isValid()) {
                kWarning() << "User invalid: " << user.loginName();
                continue;
            }

            // Display user Full Name IF available
            QString display;
            if (!user.property(KUser::FullName).toString().isEmpty())
                display = user.property(KUser::FullName).toString() + " (" + user.loginName() + ")";
            else
                display = user.loginName();

            userCB->addItem(display, user.uid());
        }

        // Show the widget
        userCB->show();
}

void AuthDialog::on_userCB_currentIndexChanged(int index)
{
    kDebug() << index;
    // itemData is Null when "Select user" is selected
    if (userCB->itemData(index).isNull()) {
        lePassword->setEnabled(false);
        lblPassword->setEnabled(false);
        cbRemember->setEnabled(false);
        cbSessionOnly->setEnabled(false);
    } else {
        lePassword->setEnabled(true);
        lblPassword->setEnabled(true);
        cbRemember->setEnabled(true);
        cbSessionOnly->setEnabled(true);
    }
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
            cbSessionOnly->hide();;
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

AuthDetails::AuthDetails(PolKitPolicyFileEntry *entry, QString appname, QWidget *parent)
        : QWidget(parent)
{
    setupUi(this);

    app_label->setText(appname);

    action_label->setText(polkit_policy_file_entry_get_action_description(entry));
    QString actionId = polkit_policy_file_entry_get_id(entry);
    action_label->setTipText(i18n("Click to edit %1", actionId));
    action_label->setUrl(actionId);

    QString vendor    = polkit_policy_file_entry_get_action_vendor(entry);
    QString vendorUrl = polkit_policy_file_entry_get_action_vendor_url(entry);
    if (!vendor.isEmpty()) {
        vendorUL->setText(vendor);
        vendorUL->setTipText(i18n("Click to open %1", vendorUrl));
        vendorUL->setUrl(vendorUrl);
    } else if (!vendorUrl.isEmpty()) {
        vendorUL->setText(vendorUrl);
        vendorUL->setTipText(i18n("Click to open %1", vendorUrl));
        vendorUL->setUrl(vendorUrl);
    } else {
        vendorL->hide();
        vendorUL->hide();
    }

    connect(vendorUL, SIGNAL(leftClickedUrl(const QString&)), SLOT(openUrl(const QString&)));
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

#include "AuthDialog.moc"
