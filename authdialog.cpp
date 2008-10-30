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
AuthDialog::AuthDialog( const QString &header, const QPixmap& pix, const QString& vendor, const KUrl& vendorUrl )
    : KDialog(0), AuthDialogUI()
{
    setButtons(Ok|Cancel);
    setCaption(header);

    QWidget* w = new QWidget(this);
    setupUi(w);
    setMainWidget(w);

    lblPixmap->setPixmap( pix );
    // TODO vendor info
    cbUsers->hide();
    lePassword->setFocus();
    setHeader( header );

    // placeholders
    i18n( "Details" );
    i18n( "Application:" );
    i18n( "Action:" );
    i18n( "Vendor:" );
}

AuthDialog::~AuthDialog()
{
}

void AuthDialog::accept()
{
    // Do nothing, do not close the dialog. This is needed so that the dialog stays
    return;
}

void AuthDialog::setHeader(const QString &header)
{
    lblHeader->setText("<h3>" + header + "</h3>");
}

void AuthDialog::setContent(const QString &msg)
{
    lblContent->setText(msg);
}

void AuthDialog::setPasswordPrompt(const QString& prompt)
{
    lblPassword->setText( prompt );
}

QString AuthDialog::password() const
{
    return lePassword->text();
}

void AuthDialog::clearPassword()
{
    lePassword->clear();
}

void AuthDialog::showKeepPassword( KeepPassword keep )
{
    switch( keep )
    {
        case KeepPasswordNo:
            cbRemember->hide();
            cbSessionOnly->hide();
            break;
        case KeepPasswordSession:
            cbRemember->setText( i18n( "Remember authorization for this session" ));
            cbRemember->show();
            cbSessionOnly->hide();
            break;
        case KeepPasswordAlways:
            cbRemember->setText( i18n( "Remember authorization" ));
            cbRemember->show();
            cbSessionOnly->show();
            break;
    }
}

KeepPassword AuthDialog::keepPassword() const
{
    if( cbRemember->isHidden()) // cannot make it keep
        return KeepPasswordNo;
    if( cbSessionOnly->isHidden()) // can keep only for session
        return cbRemember->isChecked() ? KeepPasswordSession : KeepPasswordNo;
    // can keep either way
    if( cbRemember->isChecked())
        return cbSessionOnly->isChecked() ? KeepPasswordSession : KeepPasswordAlways;
    return KeepPasswordNo;
}

