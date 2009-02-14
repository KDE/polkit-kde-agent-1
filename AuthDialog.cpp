/*  This file is part of the KDE project
    Copyright (C) 2007-2008 Gökçen Eraslan <gokcen@pardus.org.tr>
    Copyright (C) 2008 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2008 Daniel Nicoletti <dantti85-pk@yahoo.com.br>
    Copyright (C) 2008 Dario Freddi <drf54321@gmail.com>

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
#include <QPainter>

#include <KDebug>

#include <KToolInvocation>
#include <KUser>

AuthDialog::AuthDialog(PolKitPolicyFileEntry *entry, uint pid)
        : KDialog(0, Qt::Dialog | Qt::CustomizeWindowHint)
{
    setupUi(mainWidget());
    // the dialog needs to be modal to darken the parent window
    setModal(true);
    setButtons(Ok | Cancel | Details);

    kDebug() << "Getting action message...";
    QString actionMessage = QString::fromUtf8(polkit_policy_file_entry_get_action_message(entry));
    if (actionMessage.isEmpty()) {
        kWarning() << "Could not get action message for action.";
        lblHeader->hide();
    } else {
        kDebug() << "Message of action: " << actionMessage;
        lblHeader->setText("<h3>" + actionMessage + "</h3>");
        setCaption(actionMessage);
    }

    // loads the standard key icon
    QPixmap icon = KIconLoader::global()->loadIcon("dialog-password",
                                                   KIconLoader::NoGroup,
                                                   KIconLoader::SizeHuge,
                                                   KIconLoader::DefaultState);
    // create a paiter to paint the action icon over the key icon
    QPainter painter(&icon);
    const int iconSize = icon.size().width();
    // the the emblem icon to size 32
    int overlaySize = 32;
    // try to load the action icon
    const QPixmap pixmap = KIconLoader::global()->loadIcon(polkit_policy_file_entry_get_action_icon_name(entry),
                                                           KIconLoader::NoGroup,
                                                           overlaySize,
                                                           KIconLoader::DefaultState,
                                                           QStringList(),
                                                           0,
                                                           true);
    // if we're able to load the action icon paint it over the
    // key icon.
    if (!pixmap.isNull()) {
        QPoint startPoint;
        // bottom right corner
        startPoint = QPoint(iconSize - overlaySize - 2,
                            iconSize - overlaySize - 2);
        painter.drawPixmap(startPoint, pixmap);
    }

    setWindowIcon(icon);
    lblPixmap->setPixmap(icon);

    char tmp[ PATH_MAX ];
    if (polkit_sysdeps_get_exe_for_pid_with_helper(pid, tmp, sizeof(tmp) - 1) < 0) {
        m_appname.clear();
        AuthDetails *details = new AuthDetails(entry, i18nc("Unknown action", "Unknown"), this);
        setDetailsWidget(details);
    } else {
        m_appname = QString::fromUtf8(tmp);
        AuthDetails *details = new AuthDetails(entry, m_appname, this);
        setDetailsWidget(details);
    }

    userCB->hide();
    lePassword->setFocus();

    errorMessageKTW->hide();

    m_userModelSIM = new QStandardItemModel(this);
    m_userModelSIM->setSortRole(Qt::UserRole);
    userCB->setModel(m_userModelSIM);

    connect(userCB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(on_userCB_currentIndexChanged(int)));
}

AuthDialog::~AuthDialog()
{
}

void AuthDialog::accept()
{
    // Do nothing, do not close the dialog. This is needed so that the dialog stays
    return;
}

void AuthDialog::setRequest(const QString &request, bool requiresAdmin)
{
    kDebug() << request;
    QVariant userLogin;
    userLogin = m_userModelSIM->data(
                        m_userModelSIM->index(userCB->currentIndex(), 0), Qt::UserRole);
    if (request.startsWith(QLatin1String("password:"), Qt::CaseInsensitive)) {
        if (requiresAdmin) {
            if (userLogin.isNull()) {
                lblPassword->setText(i18n("Password for root:"));
            } else {
                lblPassword->setText(i18n("Password for %1:",
                                          userLogin.toString()));
            }
        } else {
            lblPassword->setText(i18n("Password:"));
        }
    } else if (request.startsWith(QLatin1String("password or swipe finger:"),
                                                        Qt::CaseInsensitive)) {
        if (requiresAdmin) {
            if (userLogin.isNull()) {
                lblPassword->setText(i18n("Password or swipe finger for root:"));
            } else {
                lblPassword->setText(i18n("Password or swipe finger for %1:",
                                          userLogin.toString()));
            }
        } else {
            lblPassword->setText(i18n("Password or swipe finger:"));
        }
    } else {
        lblPassword->setText(request);
    }

}

void AuthDialog::setOptions(KeepPassword keep, bool requiresAdmin, const QStringList &adminUsers)
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

    if (requiresAdmin) {
        // Check to see if we have the application name
        if (m_appname.isEmpty()) {
            // Check to see if the authentication is provided through group of admin users
            if (adminUsers.count()) {
                lblContent->setText(i18n("An application is attempting to perform an action that requires privileges."
                                         " Authentication as one of the users below is required to perform this action."));
                createUserCB(adminUsers);
            } else {
                lblContent->setText(i18n("An application is attempting to perform an action that requires privileges."
                                         " Authentication as the super user is required to perform this action."));
            }
        } else {
            // Check to see if the authentication is provided through group of admin users
            if (adminUsers.count()) {
                lblContent->setText(i18n("The application %1 is attempting to perform an action that requires privileges."
                                         " Authentication as one of the users below is required to perform this action.", m_appname));
                createUserCB(adminUsers);
            } else {
                lblContent->setText(i18n("The application %1 is attempting to perform an action that requires privileges."
                                         " Authentication as the super user is required to perform this action.", m_appname));
            }
        }
    } else {
        if (m_appname.isEmpty())
            lblContent->setText(i18n("An application is attempting to perform an action that requires privileges."
                                     " Authentication is required to perform this action."));
        else
            lblContent->setText(i18n("The application %1 is attempting to perform an action that requires privileges."
                                     " Authentication is required to perform this action.", m_appname));
    }
}

void AuthDialog::createUserCB(const QStringList &adminUsers)
{
    /* if we've already built the list of admin users once, then avoid
        * doing it again.. (this is mainly used when the user entered the
        * wrong password and the dialog is recycled)
        */
    if (adminUsers.count() && (userCB->count() - 1) != adminUsers.count()) {
        // Clears the combobox in the case some user be added
        m_userModelSIM->clear();

        // Adds a Dummy user
        QStandardItem *selectItem;
        m_userModelSIM->appendRow(selectItem = new QStandardItem(i18n("Select user")));
        selectItem->setSelectable(false);

        // For each user
        foreach(const QString &adminUser, adminUsers) {
            // First check to see if the user is valid
            KUser user = KUser::KUser(adminUser);
            if (!user.isValid()) {
                kWarning() << "User invalid: " << user.loginName();
                continue;
            }

            // Display user Full Name IF available
            QString display;
            if (!user.property(KUser::FullName).toString().isEmpty()) {
                display = user.property(KUser::FullName).toString() + " (" + user.loginName() + ')';
            } else {
                display = user.loginName();
            }

            QStandardItem *item = new QStandardItem(display);
            // DO NOT store UID as polkit get's confused in the case whether
            // you have another user with the same ID (ie root)
            item->setData(user.loginName(), Qt::UserRole);

            // load user icon face
            if (!user.faceIconPath().isEmpty()) {
                item->setIcon(KIcon(user.faceIconPath()));
            }
            // appends the user item
            m_userModelSIM->appendRow(item);
        }

        // Show the widget and set focus
        userCB->show();
        userCB->setFocus();
    }
}

QString AuthDialog::adminUserSelected() const
{
    QVariant userLogin;
    userLogin = m_userModelSIM->data(
                        m_userModelSIM->index(userCB->currentIndex(), 0), Qt::UserRole);
    if (userLogin.isNull()) {
        return QString();
    } else {
        return userLogin.toString();
    }
}

QString AuthDialog::selectCurrentAdminUser()
{
    KUser currentUser;
    for (int i = 1; i < userCB->count(); i++) {
        QVariant userLogin;
        userLogin = m_userModelSIM->data(
                        m_userModelSIM->index(i, 0), Qt::UserRole);
        if (userLogin.toString() == currentUser.loginName()) {
            userCB->setCurrentIndex(i);
            return currentUser.loginName();
        }
    }
    return QString();
}

void AuthDialog::on_userCB_currentIndexChanged(int index)
{
    QVariant userLogin;
    userLogin = m_userModelSIM->data(
                        m_userModelSIM->index(index, 0), Qt::UserRole);
    // itemData is Null when "Select user" is selected
    if (userLogin.isNull()) {
        lePassword->setEnabled(false);
        lblPassword->setEnabled(false);
        cbRemember->setEnabled(false);
        cbSessionOnly->setEnabled(false);
        enableButtonOk(false);
    } else {
        lePassword->setEnabled(true);
        lblPassword->setEnabled(true);
        cbRemember->setEnabled(true);
        cbSessionOnly->setEnabled(true);
        enableButtonOk(true);
        // We need this to restart the auth with the new user
        emit adminUserSelected(adminUserSelected());
        // git password label focus
        lePassword->setFocus();
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

void AuthDialog::setPasswordShowChars(bool showChars)
{
    if (showChars)
        lePassword->setEchoMode(QLineEdit::Normal);
    else
        lePassword->setEchoMode(QLineEdit::Password);
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

AuthDetails::AuthDetails(PolKitPolicyFileEntry *entry, const QString &appname, QWidget *parent)
        : QWidget(parent)
{
    setupUi(this);

    app_label->setText(appname);

    action_label->setText(QString::fromUtf8(polkit_policy_file_entry_get_action_description(entry)));
    QString actionId = polkit_policy_file_entry_get_id(entry);
    action_label->setTipText(i18n("Click to edit %1", actionId));
    action_label->setUrl(actionId);

    QString vendor    = QString::fromUtf8(polkit_policy_file_entry_get_action_vendor(entry));
    QString vendorUrl = QString::fromUtf8(polkit_policy_file_entry_get_action_vendor_url(entry));
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
