/*
 *  Copyright (C) 2021 KeePassXC Team <team@keepassxc.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KEEPASSXC_SHAREDGROUP_H
#define KEEPASSXC_SHAREDGROUP_H

#include <QMutex>
#include <QPointer>
#include <QScopedPointer>
#include <QSharedPointer>

class CompositeKey;
class Database;
class Group;
class SharedFileHandler;

class SharedGroup : public QObject
{
    Q_OBJECT;

public:
    SharedGroup(Group* group);
    ~SharedGroup() override;

    enum ShareTypeFlag
    {
        Inactive = 0x00,
        Import = 0x01,
        Export = 0x10,
        Synchronize = Import | Export
    };
    Q_DECLARE_FLAGS(ShareType, ShareTypeFlag)

    enum MessageType
    {
        Info,
        Success,
        Error
    };

    bool isValid();
    ShareType shareType();
    const Group* group();
    const Database* database();

    static bool isShared(const Group* group);

signals:
    void shareRemoved();
    void postMessage(MessageType type, QString message);

public slots:
    void sync();

private slots:
    void reset();

private:
    void readSettings();
    void readLegacySettings();
    void writeSettings();

    QScopedPointer<SharedFileHandler> m_fileHandler;

    QPointer<Group> m_group;
    ShareType m_shareType;
    QSharedPointer<CompositeKey> m_shareKey;

    bool m_inSync = false;
};

#endif // KEEPASSXC_SHAREDGROUP_H
