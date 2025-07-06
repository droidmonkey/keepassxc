/*
 *  Copyright (C) 2024 KeePassXC Team <team@keepassxc.org>
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

#ifndef KEEPASSX_DIRECTTRANSACTION_H
#define KEEPASSX_DIRECTTRANSACTION_H

#include <QDateTime>
#include <QMap>
#include <QString>
#include <QUuid>
#include <QVariant>

enum class DirectTransactionType
{
    CreateEntry,
    UpdateEntry,
    DeleteEntry,
    MoveEntry,
    CreateGroup,
    UpdateGroup,
    DeleteGroup,
    MoveGroup,
    UpdateDatabase
};

// Direct transaction target structures
struct EntryTarget
{
    QUuid entryId;
    QUuid groupId; // For move operations
};

struct GroupTarget
{
    QUuid groupId;
    QUuid parentId; // For move operations
};

struct DatabaseTarget
{
    // Database-level operations
};

// Direct transaction change structures
struct EntryChanges
{
    QMap<QString, QString> attributes;
    QMap<QString, bool> protectedAttributes;
    QMap<QString, QVariant> properties;
};

struct GroupChanges
{
    QMap<QString, QVariant> properties;
};

struct DatabaseChanges
{
    QMap<QString, QVariant> properties;
};

class DirectTransaction
{
public:
    DirectTransaction();
    DirectTransaction(DirectTransactionType type, const QString& description = QString());

    // Basic properties
    QUuid id() const
    {
        return m_id;
    }
    DirectTransactionType type() const
    {
        return m_type;
    }
    QDateTime timestamp() const
    {
        return m_timestamp;
    }
    QString description() const
    {
        return m_description;
    }

    // Target management
    void setEntryTarget(const QUuid& entryId, const QUuid& groupId = QUuid());
    void setGroupTarget(const QUuid& groupId, const QUuid& parentId = QUuid());
    void setDatabaseTarget();

    EntryTarget entryTarget() const
    {
        return m_entryTarget;
    }
    GroupTarget groupTarget() const
    {
        return m_groupTarget;
    }
    DatabaseTarget databaseTarget() const
    {
        return m_databaseTarget;
    }

    // Change management
    void setEntryChanges(const EntryChanges& changes);
    void setGroupChanges(const GroupChanges& changes);
    void setDatabaseChanges(const DatabaseChanges& changes);

    EntryChanges entryChanges() const
    {
        return m_entryChanges;
    }
    GroupChanges groupChanges() const
    {
        return m_groupChanges;
    }
    DatabaseChanges databaseChanges() const
    {
        return m_databaseChanges;
    }

    // Convenience methods for common operations
    void setEntryAttribute(const QString& key, const QString& value, bool isProtected = false);
    void setEntryProperty(const QString& property, const QVariant& value);
    void setGroupProperty(const QString& property, const QVariant& value);
    void setDatabaseProperty(const QString& property, const QVariant& value);

    // Validation
    bool isValid() const;
    QString validationError() const;

    // Utility methods
    bool hasTarget() const;
    bool hasChanges() const;

private:
    QUuid m_id;
    DirectTransactionType m_type;
    QDateTime m_timestamp;
    QString m_description;

    // Target data
    EntryTarget m_entryTarget;
    GroupTarget m_groupTarget;
    DatabaseTarget m_databaseTarget;

    // Change data
    EntryChanges m_entryChanges;
    GroupChanges m_groupChanges;
    DatabaseChanges m_databaseChanges;

    // Helper methods
    bool isEntryTransaction() const;
    bool isGroupTransaction() const;
    bool isDatabaseTransaction() const;
};

#endif // KEEPASSX_DIRECTTRANSACTION_H