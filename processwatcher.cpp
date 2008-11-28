/*
 * Copyright (C) 2008 Fredrik Höglund <fredrik@kde.org>
 *
 * Based on qprocess_unix.cpp from Qt,
 * Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 or at your option version 3 as published 
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "processwatcher.h"

#include <QSocketNotifier>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

static qint64 native_write(int fd, const char *data, qint64 len)
{
    qint64 ret = 0;
    do {
        ret = ::write(fd, data, len);
    } while (ret == -1 && errno == EINTR);
    return ret;
}

static qint64 native_read(int fd, char *data, qint64 maxlen)
{
    qint64 ret = 0;
    do {
        ret = ::read(fd, data, maxlen);
    } while (ret == -1 && errno == EINTR);
    return ret;
}

static void native_close(int fd)
{
    int ret;
    do {
        ret = ::close(fd);
    } while (ret == -1 && errno == EINTR);
}

static void native_sigaction(int signum, const struct sigaction *act,
                             struct sigaction *oldact)
{
    int ret;
    do {
        ret = ::sigaction(signum, act, oldact);
    } while (ret == -1 && errno == EINTR);
}

static int deadchild_pipe[2];
static void (*old_sigchld_handler)(int) = 0;

static void sigchld_handler(int signum)
{
    native_write(deadchild_pipe[1], "", 1);

    if (old_sigchld_handler && old_sigchld_handler != SIG_IGN)
        old_sigchld_handler(signum);
}



// ----------------------------------------------------------------------------



ProcessWatch::ProcessWatch(pid_t pid)
    : QObject(0), pid(pid)
{
#ifdef Q_OS_IRIX
    ::socketpair(AF_UNIX, SOCK_STREAM, 0, pipe);
#else
    ::pipe(deathPipe);
#endif
    ::fcntl(deathPipe[0], F_SETFD, FD_CLOEXEC);
    ::fcntl(deathPipe[1], F_SETFD, FD_CLOEXEC);
    deathNotifier = new QSocketNotifier(deathPipe[0], QSocketNotifier::Read, this);
    connect(deathNotifier, SIGNAL(activated(int)), SLOT(childDied()));
}

ProcessWatch::~ProcessWatch()
{
    native_close(deathPipe[0]);
    native_close(deathPipe[1]);
}

void ProcessWatch::childDied()
{
    // read a byte from the death pipe
    char c;
    native_read(deathPipe[0], &c, 1);

    int exitStatus;
    pid_t waitResult = 0;

    do {
        waitResult = waitpid(pid, &exitStatus, WNOHANG);
    } while ((waitResult == -1 && errno == EINTR));

    if (waitResult > 0) {
        emit terminated( pid, WEXITSTATUS(exitStatus));
    }
}


// ----------------------------------------------------------------------------


ProcessWatcher *ProcessWatcher::s_inst = 0;

ProcessWatcher *ProcessWatcher::instance()
{
    if (!s_inst)
        s_inst = new ProcessWatcher;

    return s_inst;
}

ProcessWatcher::ProcessWatcher()
{
    // Initialize the atomic ID counter
    idCounter.fetchAndStoreRelaxed(1);

    // initialize the dead child pipe and make it non-blocking. in the
    // extremely unlikely event that the pipe fills up, we do not under any
    // circumstances want to block.
    ::pipe(deadchild_pipe);
    ::fcntl(deadchild_pipe[0], F_SETFD, FD_CLOEXEC);
    ::fcntl(deadchild_pipe[1], F_SETFD, FD_CLOEXEC);
    ::fcntl(deadchild_pipe[0], F_SETFL,
            ::fcntl(deadchild_pipe[0], F_GETFL) | O_NONBLOCK);
    ::fcntl(deadchild_pipe[1], F_SETFL,
            ::fcntl(deadchild_pipe[1], F_GETFL) | O_NONBLOCK);

    // set up the SIGCHLD handler, which writes a single byte to the dead
    // child pipe every time a child dies.
    struct sigaction action;
    struct sigaction old_action;

    sigemptyset(&action.sa_mask);
    action.sa_handler = sigchld_handler;
    action.sa_flags = SA_NOCLDSTOP;
    native_sigaction(SIGCHLD, &action, &old_action);

    old_sigchld_handler = old_action.sa_handler;
    
    start();
}

ProcessWatcher::~ProcessWatcher()
{
    // notify the thread that we're shutting down.
    native_write(deadchild_pipe[1], "@", 1);
    native_close(deadchild_pipe[1]);
    wait();

    // on certain unixes, closing the reading end of the pipe will cause
    // select in run() to block forever, rather than return with EBADF.
    native_close(deadchild_pipe[0]);

    deadchild_pipe[0] = -1;
    deadchild_pipe[1] = -1;
}

int ProcessWatcher::add(ProcessWatch *watch)
{
    QMutexLocker locker(&mutex);

    int serial = idCounter.fetchAndAddRelaxed(1);
    children.insert(serial, watch);

    return serial;
}

void ProcessWatcher::remove(int serial)
{
    QMutexLocker locker(&mutex);

    ProcessWatch *watch = children.value(serial);
    if (!watch)
        return;

    children.remove(serial);
    delete watch; 
}

void ProcessWatcher::run()
{
    forever {
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(deadchild_pipe[0], &readset);

        // block forever, or until activity is detected on the dead child
        // pipe. the only other peers are the SIGCHLD signal handler, and the
        // ProcessWatcher destructor.
        int nselect = select(deadchild_pipe[0] + 1, &readset, 0, 0, 0);
        if (nselect < 0) {
            if (errno == EINTR)
                continue;
            break;
        }

        // empty only one byte from the pipe, even though several SIGCHLD
        // signals may have been delivered in the meantime, to avoid race
        // conditions.
        char c;
        if (native_read(deadchild_pipe[0], &c, 1) < 0 || c == '@')
            break;

        // catch any and all children that we can.
        catchDeadChildren();
    }
}

void ProcessWatcher::catchDeadChildren()
{
    QMutexLocker locker(&mutex);

    // try to catch all children whose pid we have registered, and whose
    // deathPipe is still valid (i.e, we have not already notified it).
    QMap<int, ProcessWatch *>::Iterator it = children.begin();
    while (it != children.end()) {
        // notify all children that they may have died. they need to run
        // waitpid() in their own thread.
        ProcessWatch *watch = it.value();
        native_write(watch->deathPipe[1], "", 1);
        ++it;
    }
}

// kate: space-indent on; indent-width 4; replace-tabs on;
