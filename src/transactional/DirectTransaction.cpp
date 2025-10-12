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

#include "DirectTransaction.h"

DirectTransaction::DirectTransaction()
    : m_id(QUuid::createUuid())
    , m_type(DirectTransactionType::UpdateEntry)
    , m_timestamp(QDateTime::currentDateTimeUtc())
{
}

DirectTransaction::DirectTransaction(DirectTransactionType type, const QString& description)
    : m_id(QUuid::createUuid())
    , m_type(type)
    , m_timestamp(QDateTime::currentDateTimeUtc())
    , m_description(description)
{
}

void DirectTransaction::setEntryTarget(const QUuid& entryId, const QUuid& groupId)
{
    m_entryTarget.entryId = entryId;
    m_entryTarget.groupId = groupId;
}

void DirectTransaction::setGroupTarget(const QUuid& groupId, const QUuid& parentId)
{
    m_groupTarget.groupId = groupId;
    m_groupTarget.parentId = parentId;
}

void DirectTransaction::setDatabaseTarget()
{
    // Database target is just a marker - no specific data needed
}

void DirectTransaction::setEntryChanges(const EntryChanges& changes)
{
    m_entryChanges = changes;
}

void DirectTransaction::setGroupChanges(const GroupChanges& changes)
{
    m_groupChanges = changes;
}

void DirectTransaction::setDatabaseChanges(const DatabaseChanges& changes)
{
    m_databaseChanges = changes;
}

void DirectTransaction::setEntryAttribute(const QString& key, const QString& value, bool isProtected)
{
    m_entryChanges.attributes[key] = value;
    m_entryChanges.protectedAttributes[key] = isProtected;
}

void DirectTransaction::setEntryProperty(const QString& property, const QVariant& value)
{
    m_entryChanges.properties[property] = value;
}

void DirectTransaction::setGroupProperty(const QString& property, const QVariant& value)
{
    m_groupChanges.properties[property] = value;
}

void DirectTransaction::setDatabaseProperty(const QString& property, const QVariant& value)
{
    m_databaseChanges.properties[property] = value;
}

bool DirectTransaction::isValid() const
{
    if (!hasTarget()) {
        return false;
    }

    if (!hasChanges() && m_type != DirectTransactionType::CreateEntry && m_type != DirectTransactionType::CreateGroup
        && m_type != DirectTransactionType::DeleteEntry && m_type != DirectTransactionType::DeleteGroup) {
        return false;
    }

    // Type-specific validation
    if (isEntryTransaction() && m_entryTarget.entryId.isNull()) {
        return false;
    }

    if (isGroupTransaction() && m_groupTarget.groupId.isNull()) {
        return false;
    }

    return true;
}

QString DirectTransaction::validationError() const
{
    if (!hasTarget()) {
        return "Transaction has no target";
    }

    if (!hasChanges() && m_type != DirectTransactionType::CreateEntry && m_type != DirectTransactionType::CreateGroup
        && m_type != DirectTransactionType::DeleteEntry && m_type != DirectTransactionType::DeleteGroup) {
        return "Transaction has no changes";
    }

    if (isEntryTransaction() && m_entryTarget.entryId.isNull()) {
        return "Entry transaction requires valid entry ID";
    }

    if (isGroupTransaction() && m_groupTarget.groupId.isNull()) {
        return "Group transaction requires valid group ID";
    }

    return QString();
}

bool DirectTransaction::hasTarget() const
{
    return !m_entryTarget.entryId.isNull() || !m_groupTarget.groupId.isNull() || isDatabaseTransaction();
}

bool DirectTransaction::hasChanges() const
{
    return !m_entryChanges.attributes.isEmpty() || !m_entryChanges.properties.isEmpty()
           || !m_groupChanges.properties.isEmpty() || !m_databaseChanges.properties.isEmpty();
}

bool DirectTransaction::isEntryTransaction() const
{
    return m_type == DirectTransactionType::CreateEntry || m_type == DirectTransactionType::UpdateEntry
           || m_type == DirectTransactionType::DeleteEntry || m_type == DirectTransactionType::MoveEntry;
}

bool DirectTransaction::isGroupTransaction() const
{
    return m_type == DirectTransactionType::CreateGroup || m_type == DirectTransactionType::UpdateGroup
           || m_type == DirectTransactionType::DeleteGroup || m_type == DirectTransactionType::MoveGroup;
}

bool DirectTransaction::isDatabaseTransaction() const
{
    return m_type == DirectTransactionType::UpdateDatabase;
}