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

#ifndef PROCESSWATCHER_H
#define PROCESSWATCHER_H

#include <QThread>
#include <QMutex>
#include <QMap>
#include <QAtomic>

struct _PolKitGrant;
typedef struct _PolKitGrant PolKitGrant;

class ProcessInfo;
class QSocketNotifier;

/**
 * This class represents a child process being watched.
 */
class ProcessWatch : public QObject
{
    Q_OBJECT

public:
    ProcessWatch(PolKitGrant *polkit_grant, pid_t pid);
    ~ProcessWatch();

private slots:
    void childDied();

private:
    PolKitGrant *polkit_grant;
    QSocketNotifier *deathNotifier;
    int deathPipe[2];
    pid_t pid;

friend class ProcessWatcher;
};


/**
 * This class watches child processes spawned by PolicyKitGrant
 * on its behalf, and notifies it when a process dies.
 *
 * Only one instance of this class should be created.
 */
class ProcessWatcher : public QThread
{
public:
    ~ProcessWatcher();

    static ProcessWatcher *instance();

    /**
     * Adds a watch and returns a unique identifier for this watch.
     * Ownership is transferred to the process watcher.
     */
    int add(ProcessWatch *watch);

    /**
     * Removes and deletes the watch identified by @p id
     */
    void remove(int id);

private:
    ProcessWatcher();
    void run();
    void catchDeadChildren();

private:
    QMutex mutex;
    QAtomicInt idCounter;
    QMap<int, ProcessWatch *> children;
    static ProcessWatcher *s_inst;
};

#endif
