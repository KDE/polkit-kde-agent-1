// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include <KLocalizedString>
#include <KCrash>

#include <QApplication>
#include <QDebug>
#include <QSessionManager>
#include <QSharedPointer>
#include <QIcon>

#include <PolkitQt1/Subject>

#include "listener.h"

#include <malloc.h>

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

static char **s_argv;

// the QML engine is very bad at freeing memory, even if we delete it.
// we gotta do this ugly reexec hack in order to release adequate memory.
// apparently it might be a glibc allocator thing.
// would be interested in seeing how this fares with another allocator/libc.
void reexec()
{
    QCoreApplication::exit(0);
    execvp("/proc/self/exe", s_argv);
}

int main(int argc, char *argv[])
{
    s_argv = argv;

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