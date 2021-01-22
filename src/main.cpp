// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include <KLocalizedString>
#include <KCrash>

#include <QApplication>
#include <QSessionManager>
#include <QSharedPointer>
#include <QIcon>

#include <PolkitQt1/Subject>

#include "listener.h"

#if defined(_LINUX)
#include <sys/prctl.h>
#elif defined(_FREEBSD)
#include <sys/procctl.h>
#include <unistd.h>
#endif

void disableCoreDumping()
{
#if defined(_LINUX)
    prctl(PR_SET_DUMPABLE, 0);
#elif defined(_FREEBSD)
    int mode = PROC_TRACE_CTL_DISABLE;
    procctl(P_PID, getpid(), PROC_TRACE_CTL, &mode);
#else

#warning "Platform not known; please add support for disabling coredumps to polkit-kde-agent-1"

#endif
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    app.setAttribute(Qt::AA_EnableHighDpiScaling, true);

    KLocalizedString::setApplicationDomain("polkit-kde-authentication-agent-1");

    KCrash::setFlags(KCrash::AutoRestart);

    Listener listener(&app);
    listener.listen();

    return app.exec();
}