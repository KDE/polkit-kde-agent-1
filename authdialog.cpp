/*  This file is part of the KDE project
    Copyright (C) 2007-2008 Gökçen Eraslan <gokcen@pardus.org.tr>
    Copyright (C) 2008 Dirk Mueller <mueller@kde.org>

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
#include "authdialog.moc"

#include <qlabel.h>
#include <qstring.h>
#include <qnamespace.h>
#include <qcheckbox.h>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <kdebug.h>

/* 
 *  Constructs a AuthDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AuthDialog::AuthDialog( const QString &header,
            PolKitResult type)
    : QDialog(0), AuthDialogUI()
{
    setupUi(this);

    if (type == POLKIT_RESULT_UNKNOWN || \
            type == POLKIT_RESULT_NO || \
            type == POLKIT_RESULT_YES || \
            type == POLKIT_RESULT_N_RESULTS )
        kDebug() << "Unexpected PolkitResult type sent: " << polkit_result_to_string_representation(type);

    KIconLoader* iconloader = KIconLoader::global();
    lblPixmap->setPixmap(iconloader->loadIcon("lock", KIconLoader::Desktop));
    pbOK->setIconSet(iconloader->loadIconSet("ok", KIconLoader::Small, 0, false));
    pbCancel->setIconSet(iconloader->loadIconSet("cancel", KIconLoader::Small, 0, false));

    cbUsers->hide();

    setType(type);
    setHeader(header);
    setContent();
}

AuthDialog::~AuthDialog()
{
}

void AuthDialog::setHeader(const QString &header)
{
    lblHeader->setText("<h3>" + header + "</h3>");
}

void AuthDialog::setContent(const QString &msg)
{
    lblContent->setText(msg);
}

// set content according to m_type, that is a PolKitResult 
void AuthDialog::setContent()
{
    QString msg;
    switch(m_type)
    {
        //TODO: Authentication as one of the users below...
        case POLKIT_RESULT_ONLY_VIA_ADMIN_AUTH:
        case POLKIT_RESULT_ONLY_VIA_ADMIN_AUTH_KEEP_SESSION:
        case POLKIT_RESULT_ONLY_VIA_ADMIN_AUTH_KEEP_ALWAYS:
            msg = i18n("An application is attempting to perform an action that requires privileges."
                    " Authentication as the super user is required to perform this action.");
            break;
        default:
            msg = i18n("An application is attempting to perform an action that requires privileges."
                    " Authentication is required to perform this action.");

    }
    lblContent->setText(msg);
}


void AuthDialog::showUsersCombo()
{
    cbUsers->show();
}

void AuthDialog::hideUsersCombo()
{
    cbUsers->hide();
}

void AuthDialog::setPasswordFor(bool set, const QString& user)
{
    if (set)
        lblPassword->setText(i18n("Password for root") + ":");
    else if (!user.isEmpty())
        lblPassword->setText(i18n("Password for user(%1)").arg(user) + ":");
    else
        lblPassword->setText(i18n("Password") + ":");
}

const char* AuthDialog::getPass()
{
    return lePassword->text();
}

void AuthDialog::setType(PolKitResult res)
{
    if (res == POLKIT_RESULT_ONLY_VIA_ADMIN_AUTH || \
            res == POLKIT_RESULT_ONLY_VIA_ADMIN_AUTH_KEEP_SESSION || \
            res == POLKIT_RESULT_ONLY_VIA_ADMIN_AUTH_KEEP_ALWAYS)
        setPasswordFor(true);

    if (res == POLKIT_RESULT_ONLY_VIA_SELF_AUTH || \
            res == POLKIT_RESULT_ONLY_VIA_SELF_AUTH_KEEP_SESSION || \
            res == POLKIT_RESULT_ONLY_VIA_SELF_AUTH_KEEP_ALWAYS)
        setPasswordFor(false);

    if (res == POLKIT_RESULT_ONLY_VIA_ADMIN_AUTH || res == POLKIT_RESULT_ONLY_VIA_SELF_AUTH)
    {
        cbRemember->hide();
        cbSession->hide();
    }

    if (res == POLKIT_RESULT_ONLY_VIA_ADMIN_AUTH_KEEP_SESSION || res == POLKIT_RESULT_ONLY_VIA_SELF_AUTH_KEEP_SESSION)
        cbRemember->hide();

    m_type = res;
}
