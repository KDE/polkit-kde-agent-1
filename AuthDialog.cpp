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

#include <QtCore/QProcess>
#include <QtGui/QPainter>
#include <QtGui/QStandardItemModel>

#include <KDebug>

#include <KToolInvocation>
#include <KUser>

#include <PolkitQt1/Authority>
#include <PolkitQt1/Details>

Q_DECLARE_METATYPE(PolkitQt1::Identity *);

AuthDialog::AuthDialog(const QString &actionId,
                       const QString &message,
                       const QString &iconName,
                       PolkitQt1::Details *details,
                       QList<PolkitQt1::Identity *> identities)
        : KDialog(0, Qt::Dialog)
{
    qRegisterMetaType<PolkitQt1::Identity *> ("PolkitQt1::Identity *");
    setupUi(mainWidget());
    // the dialog needs to be modal to darken the parent window
    setModal(true);
    setButtons(Ok | Cancel | Details);

    if (message.isEmpty()) {
        kWarning() << "Could not get action message for action.";
        lblHeader->hide();
    } else {
        kDebug() << "Message of action: " << message;
        lblHeader->setText("<h3>" + message + "</h3>");
        setCaption(message);
    }

    KIcon icon = KIcon("dialog-password", 0, QStringList() << iconName);

    setWindowIcon(icon);
    lblPixmap->setPixmap(icon.pixmap(QSize(KIconLoader::SizeHuge, KIconLoader::SizeHuge)));

    // find action description for actionId
    foreach(PolkitQt1::ActionDescription *desc, PolkitQt1::Authority::instance()->enumerateActionsSync()) {
        if (desc && actionId == desc->actionId()) {
            m_actionDescription = desc;
            kDebug() << "Action description has been found" ;
            break;
        }
    }

    AuthDetails *detailsDialog = new AuthDetails(details, m_actionDescription, m_appname, this);
    setDetailsWidget(detailsDialog);

    userCB->hide();
    lePassword->setFocus();

    errorMessageKTW->hide();

    m_userModelSIM = new QStandardItemModel(this);
    m_userModelSIM->setSortRole(Qt::UserRole);

    // If there is more than 1 identity we will show the combobox for user selection
    if (identities.size() > 1) {
        userCB->setModel(m_userModelSIM);

        connect(userCB, SIGNAL(currentIndexChanged(int)),
                this, SLOT(on_userCB_currentIndexChanged(int)));

        createUserCB(identities);
    } else {
        userCB->setCurrentIndex(0);
        QStandardItem *item = new QStandardItem("");
        item->setData(qVariantFromValue<PolkitQt1::Identity *> (identities[0]), Qt::UserRole);
        m_userModelSIM->appendRow(item);
    }
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
    PolkitQt1::Identity *identity = adminUserSelected();
    if (request.startsWith(QLatin1String("password:"), Qt::CaseInsensitive)) {
        if (requiresAdmin) {
            if (identity == NULL) {
                lblPassword->setText(i18n("Password for root:"));
            } else {
                lblPassword->setText(i18n("Password for %1:",
                                          identity->toString().remove("unix-user:")));
            }
        } else {
            lblPassword->setText(i18n("Password:"));
        }
    } else if (request.startsWith(QLatin1String("password or swipe finger:"),
                                  Qt::CaseInsensitive)) {
        if (requiresAdmin) {
            if (identity == NULL) {
                lblPassword->setText(i18n("Password or swipe finger for root:"));
            } else {
                lblPassword->setText(i18n("Password or swipe finger for %1:",
                                          identity->toString().remove("unix-user:")));
            }
        } else {
            lblPassword->setText(i18n("Password or swipe finger:"));
        }
    } else {
        lblPassword->setText(request);
    }

}

void AuthDialog::setOptions()
{
    lblContent->setText(i18n("An application is attempting to perform an action that requires privileges."
                             " Authentication is required to perform this action."));
}

void AuthDialog::createUserCB(QList<PolkitQt1::Identity *> identities)
{
    /* if we've already built the list of admin users once, then avoid
        * doing it again.. (this is mainly used when the user entered the
        * wrong password and the dialog is recycled)
        */
    if (identities.count() && (userCB->count() - 1) != identities.count()) {
        // Clears the combobox in the case some user be added
        m_userModelSIM->clear();

        // Adds a Dummy user
        QStandardItem *selectItem;
        m_userModelSIM->appendRow(selectItem = new QStandardItem(i18n("Select User")));
        selectItem->setSelectable(false);
        selectItem->setData(QVariant(), Qt::UserRole);

        // For each user
        foreach(PolkitQt1::Identity *identity, identities) {
            // First check to see if the user is valid
            qDebug() << "User: " << identity;
            KUser user = KUser::KUser(identity->toString().remove("unix-user:"));
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
            item->setData(qVariantFromValue<PolkitQt1::Identity *> (identity), Qt::UserRole);

            // load user icon face
            if (!user.faceIconPath().isEmpty()) {
                item->setIcon(KIcon(user.faceIconPath()));
            } else {
                item->setIcon(KIcon("user-identity"));
            }
            // appends the user item
            m_userModelSIM->appendRow(item);
        }

        // Show the widget and set focus
        userCB->show();
        userCB->setFocus();
    }
}

PolkitQt1::Identity *AuthDialog::adminUserSelected()
{
    return qVariantValue<PolkitQt1::Identity *> (m_userModelSIM->data(
                m_userModelSIM->index(userCB->currentIndex(), 0), Qt::UserRole));
}

void AuthDialog::on_userCB_currentIndexChanged(int /*index*/)
{
    PolkitQt1::Identity *identity = adminUserSelected();
    // itemData is Null when "Select user" is selected
    if (identity == NULL) {
        lePassword->setEnabled(false);
        lblPassword->setEnabled(false);
        enableButtonOk(false);
    } else {
        lePassword->setEnabled(true);
        lblPassword->setEnabled(true);
        enableButtonOk(true);
        // We need this to restart the auth with the new user
        emit adminUserSelected(identity);
        // git password label focus
        lePassword->setFocus();
    }
}

QString AuthDialog::password() const
{
    return lePassword->text();
}

void AuthDialog::authenticationFailure()
{
    lePassword->clear();
    errorMessageKTW->setText(i18n("Authentication failure, please try again."), KTitleWidget::ErrorMessage);
    QFont bold = font();
    bold.setBold(true);
    lblPassword->setFont(bold);
    lePassword->clear();
    lePassword->setFocus();
}

AuthDetails::AuthDetails(PolkitQt1::Details *details,
                         PolkitQt1::ActionDescription *actionDescription,
                         const QString &appname,
                         QWidget *parent)
        : QWidget(parent)
{
    setupUi(this);

    app_label->setText(appname);

    foreach(const QString &key, details->keys()) {
        int row = gridLayout->rowCount() + 1;

        QLabel *keyLabel = new QLabel(this);
        keyLabel->setText(i18nc("%1 is the name of a detail about the current action "
                                "provided by polkit", "%1:", key));
        gridLayout->addWidget(keyLabel, row, 0);

        QLabel *valueLabel = new QLabel(this);
        valueLabel->setText(details->lookup(key));
        gridLayout->addWidget(valueLabel, row, 1);
    }

    if (actionDescription) {
        action_label->setText(actionDescription->description());

        action_label->setTipText(i18n("Click to edit %1", actionDescription->actionId()));
        action_label->setUrl(actionDescription->actionId());

        QString vendor    = actionDescription->vendorName();
        QString vendorUrl = actionDescription->vendorUrl();

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
