/***************************************************************************
* Copyright (c) 2013 Abdurrahman AVCI <abdurrahmanavci@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
***************************************************************************/

#include "Session.h"

#include "Authenticator.h"
#include "Configuration.h"

#include <QDebug>

#include <grp.h>
#include <pwd.h>
#include <unistd.h>

namespace SDDM {
    Session::Session(const QString &name, QObject *parent) : QProcess(parent), m_name(name) {
    }

    const QString &Session::name() const {
        return m_name;
    }

    void Session::setUser(const QString &user) {
        m_user = user;
    }

    void Session::setDir(const QString &dir) {
        m_dir = dir;
    }

    void Session::setUid(int uid) {
        m_uid = uid;
    }

    void Session::setGid(int gid) {
        m_gid = gid;
    }

    void Session::setupChildProcess() {
        if (Configuration::instance()->testing)
            return;

        Authenticator *authenticator = qobject_cast<Authenticator *>(parent());

        if (initgroups(qPrintable(m_user), m_gid)) {
            qCritical() << " DAEMON: Failed to initialize user groups.";

            // emit signal
            emit finished(EXIT_FAILURE, QProcess::NormalExit);

            // exit
            exit(EXIT_FAILURE);
        }

        if (setgid(m_gid)) {
            qCritical() << " DAEMON: Failed to set group id.";

            // emit signal
            emit finished(EXIT_FAILURE, QProcess::NormalExit);

            // exit
            exit(EXIT_FAILURE);
        }

        if (setuid(m_uid)) {
            qCritical() << " DAEMON: Failed to set user id.";

            // emit signal
            emit finished(EXIT_FAILURE, QProcess::NormalExit);

            // exit
            exit(EXIT_FAILURE);

        }

        // add cookie
        authenticator->addCookie(QString("%1/.Xauthority").arg(m_dir));

        // change to user home dir
        if (chdir(qPrintable(m_dir))) {
            qCritical() << " DAEMON: Failed to change dir to user home.";

            // emit signal
            emit finished(EXIT_FAILURE, QProcess::NormalExit);

            // exit
            exit(EXIT_FAILURE);
        }

        // copy environment to pam environment
        for (int i = 0; environ[i] != nullptr; ++i)
            authenticator->putenv(environ[i]);
    }
}