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

#include <QProcess>
#include <QPainter>
#include <QStandardItemModel>
#include <QDebug>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QUrl>

#include <KWindowSystem>
#include <KIconLoader>
#include <KUser>

#include <PolkitQt1/Authority>
#include <PolkitQt1/Details>

AuthDialog::AuthDialog(const QString &actionId,
                       const QString &message,
                       const QString &iconName,
                       const PolkitQt1::Details &details,
                       const PolkitQt1::Identity::List &identities,
                       WId parent)
    : QDialog(0)
{
    // KAuth is able to circumvent polkit's limitations, and manages to send the wId to the auth agent.
    // If we received it, we use KWindowSystem to associate this dialog correctly.
    if (parent > 0) {
        qDebug() << "Associating the dialog with " << parent << " this dialog is " << winId();

        // Set the parent
        KWindowSystem::setMainWindow(this, parent);

        // Set modal
        setWindowModality(Qt::ApplicationModal);

        // raise on top
        activateWindow();
        raise();
    }

    setupUi(this);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &AuthDialog::okClicked);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QString detailsButtonText = i18n("Details");
    QPushButton* detailsButton = new QPushButton(detailsButtonText + " >>");
    detailsButton->setIcon(QIcon::fromTheme("help-about"));
    detailsButton->setCheckable(true);
    connect(detailsButton, &QAbstractButton::toggled, this, [=](bool toggled) {
        detailsWidgetContainer->setVisible(toggled);
        if (toggled) {
            detailsButton->setText(detailsButtonText + " <<");
        } else {
               detailsButton->setText(detailsButtonText + " >>");
        }
        adjustSize();
    });
    buttonBox->addButton(detailsButton, QDialogButtonBox::HelpRole);
    detailsWidgetContainer->hide();

    setWindowTitle(i18n("Authentication Required"));

    if (message.isEmpty()) {
        qWarning() << "Could not get action message for action.";
        lblHeader->hide();
    } else {
        qDebug() << "Message of action: " << message;
        lblHeader->setText("<h3>" + message + "</h3>");
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
    // the emblem icon to size 32
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
            qDebug() << "Action description has been found" ;
            break;
        }
    }

    AuthDetails *detailsDialog = new AuthDetails(details, m_actionDescription, m_appname, this);
    detailsWidgetContainer->layout()->addWidget(detailsDialog);

    userCB->hide();
    lePassword->setFocus();

    errorMessageWidget->setMessageType(KMessageWidget::Error);
    errorMessageWidget->hide();

    // If there is more than 1 identity we will show the combobox for user selection
    if (identities.size() > 1) {
        connect(userCB, SIGNAL(currentIndexChanged(int)),
                this, SLOT(on_userCB_currentIndexChanged(int)));

        createUserCB(identities);
    } else {
        userCB->addItem("", identities[0].toString());
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
    qDebug() << request;
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
        userCB->addItem(i18n("Select User"), QString());
        qobject_cast<QStandardItemModel *>(userCB->model())->item(userCB->count()-1)->setEnabled(false);

        // For each user
        int index = 1; // Start at 1 because of the "Select User" entry
        int currentUserIndex = -1;
        const KUser currentUser;
        foreach(const PolkitQt1::Identity &identity, identities) {
            // First check to see if the user is valid
            qDebug() << "User: " << identity.toString();
            const KUser user(identity.toString().remove("unix-user:"));
            if (!user.isValid()) {
                qWarning() << "User invalid: " << user.loginName();
                continue;
            }

            // Display user Full Name IF available
            QString display;
            if (!user.property(KUser::FullName).toString().isEmpty()) {
                display = i18nc("%1 is the full user name, %2 is the user login name", "%1 (%2)", user.property(KUser::FullName).toString(), user.loginName());
            } else {
                display = user.loginName();
            }

            QIcon icon;
            // load user icon face
            if (!user.faceIconPath().isEmpty()) {
                icon = QIcon(user.faceIconPath());
            } else {
                icon = QIcon::fromTheme("user-identity");
            }
            // appends the user item
            userCB->addItem(icon, display, identity.toString());

            if (user == currentUser) {
                currentUserIndex = index;
            }
            ++index;
        }

        // Show the widget and set focus
        if (currentUserIndex != -1) {
            userCB->setCurrentIndex(currentUserIndex);
        }
        userCB->show();
    }
}

PolkitQt1::Identity AuthDialog::adminUserSelected() const
{
    if (userCB->currentIndex() == -1)
        return PolkitQt1::Identity();

    const QString id = userCB->currentData().toString();
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
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    } else {
        lePassword->setEnabled(true);
        lblPassword->setEnabled(true);
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
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
    errorMessageWidget->setText(i18n("Authentication failure, please try again."));
    errorMessageWidget->animatedShow();

    QFont bold = font();
    bold.setBold(true);
    lblPassword->setFont(bold);
    lePassword->setEnabled(true);
    lePassword->clear();
    lePassword->setFocus();
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

    connect(vendorUL, SIGNAL(leftClickedUrl(QString)), SLOT(openUrl(QString)));
    connect(action_label, SIGNAL(leftClickedUrl(QString)), SLOT(openAction(QString)));
}

void AuthDetails::openUrl(const QString& url)
{
    QDesktopServices::openUrl(QUrl(url));
}

void AuthDetails::openAction(const QString &url)
{
    // FIXME what's this? :)
    QProcess::startDetached("polkit-kde-authorization", QStringList() << url);
}
