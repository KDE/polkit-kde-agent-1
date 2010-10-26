/*  This file is part of the KDE project
    Copyright (C) 2007-2008 Gökçen Eraslan <gokcen@pardus.org.tr>
    Copyright (C) 2008 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2008 Daniel Nicoletti <dantti85-pk@yahoo.com.br>
    Copyright (C) 2008-2010 Dario Freddi <drf@kde.org>

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

#include <KWindowSystem>
#include <KNotification>

AuthDialog::AuthDialog(const QString &actionId,
                       const QString &message,
                       const QString &iconName,
                       const PolkitQt1::Details &details,
                       const PolkitQt1::Identity::List &identities,
                       WId parent)
        : KDialog(0)
{
    // KAuth is able to circumvent polkit's limitations, and manages to send the wId to the auth agent.
    // If we received it, we use KWindowSystem to associate this dialog correctly.
    if (parent > 0) {
        kDebug() << "Associating the dialog with " << parent << " this dialog is " << winId();

        // Set the parent
        KWindowSystem::setMainWindow(this, parent);

        // Set modal
        KWindowSystem::setState(winId(), NET::Modal);
    }

    setupUi(mainWidget());
    setButtons(Ok | Cancel | Details);

    if (message.isEmpty()) {
        kWarning() << "Could not get action message for action.";
        lblHeader->hide();
    } else {
        kDebug() << "Message of action: " << message;
        lblHeader->setText("<h3>" + message + "</h3>");
        setCaption(message);
        m_message = message;
    }

    // loads the standard key icon
    QPixmap icon = KIconLoader::global()->loadIcon("dialog-password",
                                                    KIconLoader::NoGroup,
                                                    KIconLoader::SizeHuge,
                                                    KIconLoader::DefaultState);
    // create a painter to paint the action icon over the key icon
    QPainter painter(&icon);
    const int iconSize = icon.size().width();
    // the the emblem icon to size 32
    int overlaySize = 32;
    // try to load the action icon
    const QPixmap pixmap = KIconLoader::global()->loadIcon(iconName,
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

    // find action description for actionId
    foreach(const PolkitQt1::ActionDescription &desc, PolkitQt1::Authority::instance()->enumerateActionsSync()) {
        if (actionId == desc.actionId()) {
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

    // If there is more than 1 identity we will show the combobox for user selection
    if (identities.size() > 1) {
        connect(userCB, SIGNAL(currentIndexChanged(int)),
                this, SLOT(on_userCB_currentIndexChanged(int)));

        createUserCB(identities);
    } else {
        userCB->addItem("", QVariant(identities[0].toString()));
        userCB->setCurrentIndex(0);
    }
}

AuthDialog::~AuthDialog()
{
}

void AuthDialog::accept()
{
    // Do nothing, do not close the dialog. This is needed so that the dialog stays
    lePassword->setEnabled(false);
    return;
}

void AuthDialog::setRequest(const QString &request, bool requiresAdmin)
{
    kDebug() << request;
    PolkitQt1::Identity identity = adminUserSelected();
    if (request.startsWith(QLatin1String("password:"), Qt::CaseInsensitive)) {
        if (requiresAdmin) {
            if (!identity.isValid()) {
                lblPassword->setText(i18n("Password for root:"));
            } else {
                lblPassword->setText(i18n("Password for %1:",
                                          identity.toString().remove("unix-user:")));
            }
        } else {
            lblPassword->setText(i18n("Password:"));
        }
    } else if (request.startsWith(QLatin1String("password or swipe finger:"),
                                  Qt::CaseInsensitive)) {
        if (requiresAdmin) {
            if (!identity.isValid()) {
                lblPassword->setText(i18n("Password or swipe finger for root:"));
            } else {
                lblPassword->setText(i18n("Password or swipe finger for %1:",
                                          identity.toString().remove("unix-user:")));
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

void AuthDialog::createUserCB(const PolkitQt1::Identity::List &identities)
{
    /* if we've already built the list of admin users once, then avoid
        * doing it again.. (this is mainly used when the user entered the
        * wrong password and the dialog is recycled)
        */
    if (identities.count() && (userCB->count() - 1) != identities.count()) {
        // Clears the combobox in the case some user be added
        userCB->clear();

        // Adds a Dummy user
        userCB->addItem(i18n("Select User"), qVariantFromValue<QString> (QString()));
        qobject_cast<QStandardItemModel *>(userCB->model())->item(userCB->count()-1)->setEnabled(false);

        // For each user
        foreach(const PolkitQt1::Identity &identity, identities) {
            // First check to see if the user is valid
            kDebug() << "User: " << identity.toString();
            KUser user(identity.toString().remove("unix-user:"));
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

            KIcon icon;
            // load user icon face
            if (!user.faceIconPath().isEmpty()) {
                icon = KIcon(user.faceIconPath());
            } else {
                icon = KIcon("user-identity");
            }
            // appends the user item
            userCB->addItem(icon, display, qVariantFromValue<QString> (identity.toString()));
        }

        // Show the widget and set focus
        userCB->show();
        userCB->setFocus();
    }
}

PolkitQt1::Identity AuthDialog::adminUserSelected() const
{
    if (userCB->currentIndex() == -1)
        return PolkitQt1::Identity();

    QString id = userCB->itemData(userCB->currentIndex()).toString();
    if (id.isEmpty())
        return PolkitQt1::Identity();
    return PolkitQt1::Identity::fromString(id);
}

void AuthDialog::on_userCB_currentIndexChanged(int /*index*/)
{
    PolkitQt1::Identity identity = adminUserSelected();
    // itemData is Null when "Select user" is selected
    if (!identity.isValid()) {
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
    errorMessageKTW->setText(i18n("Authentication failure, please try again."), KTitleWidget::ErrorMessage);
    QFont bold = font();
    bold.setBold(true);
    lblPassword->setFont(bold);
    lePassword->setEnabled(true);
    lePassword->clear();
    lePassword->setFocus();
}

void AuthDialog::showEvent(QShowEvent *event)
{
    KDialog::showEvent(event);
    if (winId() != KWindowSystem::activeWindow())
    {
        KNotification *notification = new KNotification("authenticate", this,
                                                        KNotification::Persistent | KNotification::CloseWhenWidgetActivated);
        kDebug() << "Notificate: " << notification->eventId();
        notification->setText(m_message);
        QPixmap icon = KIconLoader::global()->loadIcon("dialog-password",
                                                        KIconLoader::NoGroup,
                                                        KIconLoader::SizeHuge,
                                                        KIconLoader::DefaultState);
        notification->setPixmap(icon);
        notification->setActions(QStringList() << i18n("Switch to dialog") << i18n("Cancel"));

        connect(notification, SIGNAL(activated(unsigned int)), this, SLOT(notificationActivated(unsigned int)));
        notification->sendEvent();
    }

}

void AuthDialog::notificationActivated(unsigned int action)
{
    kDebug() << "notificationActivated: " << action;
    if (action == 1)
    {
        KWindowSystem::forceActiveWindow(winId());
    }
}

AuthDetails::AuthDetails(const PolkitQt1::Details &details,
                         const PolkitQt1::ActionDescription &actionDescription,
                         const QString &appname,
                         QWidget *parent)
        : QWidget(parent)
{
    setupUi(this);

    app_label->setText(appname);

    foreach(const QString &key, details.keys()) { //krazy:exclude=foreach (Details is not a map/hash, but rather a method)
        int row = gridLayout->rowCount() + 1;

        QLabel *keyLabel = new QLabel(this);
        keyLabel->setText(i18nc("%1 is the name of a detail about the current action "
                                "provided by polkit", "%1:", key));
        gridLayout->addWidget(keyLabel, row, 0);

        QLabel *valueLabel = new QLabel(this);
        valueLabel->setText(details.lookup(key));
        gridLayout->addWidget(valueLabel, row, 1);
    }

    action_label->setText(actionDescription.description());

    action_label->setTipText(i18n("Click to edit %1", actionDescription.actionId()));
    action_label->setUrl(actionDescription.actionId());

    QString vendor    = actionDescription.vendorName();
    QString vendorUrl = actionDescription.vendorUrl();

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
