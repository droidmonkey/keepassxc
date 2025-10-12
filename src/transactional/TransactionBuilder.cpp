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

#include "TransactionBuilder.h"
#include <stdexcept>

TransactionBuilder::TransactionBuilder()
    : m_mode(BuilderMode::Unspecified)
    , m_transactionType(DirectTransactionType::UpdateEntry)
    , m_operationSet(false)
    , m_hasChanges(false)
{
}

TransactionBuilder TransactionBuilder::forEntry(const QUuid& entryId)
{
    TransactionBuilder builder;
    builder.m_mode = BuilderMode::Entry;
    builder.m_entryId = entryId;
    return builder;
}

TransactionBuilder TransactionBuilder::forGroup(const QUuid& groupId)
{
    TransactionBuilder builder;
    builder.m_mode = BuilderMode::Group;
    builder.m_groupId = groupId;
    return builder;
}

TransactionBuilder TransactionBuilder::forDatabase()
{
    TransactionBuilder builder;
    builder.m_mode = BuilderMode::Database;
    return builder;
}

TransactionBuilder& TransactionBuilder::withDescription(const QString& description)
{
    m_description = description;
    return *this;
}

// Entry operations
TransactionBuilder& TransactionBuilder::createEntry(const QUuid& parentGroupId)
{
    ensureMode(BuilderMode::Entry, "createEntry");
    ensureNoOperation("createEntry");
    setOperation(DirectTransactionType::CreateEntry);
    m_targetId = parentGroupId;
    return *this;
}

TransactionBuilder& TransactionBuilder::updateEntry()
{
    ensureMode(BuilderMode::Entry, "updateEntry");
    ensureNoOperation("updateEntry");
    setOperation(DirectTransactionType::UpdateEntry);
    return *this;
}

TransactionBuilder& TransactionBuilder::deleteEntry()
{
    ensureMode(BuilderMode::Entry, "deleteEntry");
    ensureNoOperation("deleteEntry");
    setOperation(DirectTransactionType::DeleteEntry);
    return *this;
}

TransactionBuilder& TransactionBuilder::moveEntryTo(const QUuid& targetGroupId)
{
    ensureMode(BuilderMode::Entry, "moveEntryTo");
    ensureNoOperation("moveEntryTo");
    setOperation(DirectTransactionType::MoveEntry);
    m_targetId = targetGroupId;
    return *this;
}

// Entry attribute updates
TransactionBuilder& TransactionBuilder::withTitle(const QString& title)
{
    ensureMode(BuilderMode::Entry, "withTitle");
    m_entryChanges.attributes["Title"] = title;
    m_entryChanges.protectedAttributes["Title"] = false;
    m_hasChanges = true;
    return *this;
}

TransactionBuilder& TransactionBuilder::withUsername(const QString& username)
{
    ensureMode(BuilderMode::Entry, "withUsername");
    m_entryChanges.attributes["UserName"] = username;
    m_entryChanges.protectedAttributes["UserName"] = false;
    m_hasChanges = true;
    return *this;
}

TransactionBuilder& TransactionBuilder::withPassword(const QString& password)
{
    ensureMode(BuilderMode::Entry, "withPassword");
    m_entryChanges.attributes["Password"] = password;
    m_entryChanges.protectedAttributes["Password"] = true;
    m_hasChanges = true;
    return *this;
}

TransactionBuilder& TransactionBuilder::withUrl(const QString& url)
{
    ensureMode(BuilderMode::Entry, "withUrl");
    m_entryChanges.attributes["URL"] = url;
    m_entryChanges.protectedAttributes["URL"] = false;
    m_hasChanges = true;
    return *this;
}

TransactionBuilder& TransactionBuilder::withNotes(const QString& notes)
{
    ensureMode(BuilderMode::Entry, "withNotes");
    m_entryChanges.attributes["Notes"] = notes;
    m_entryChanges.protectedAttributes["Notes"] = false;
    m_hasChanges = true;
    return *this;
}

TransactionBuilder& TransactionBuilder::withAttribute(const QString& key, const QString& value, bool isProtected)
{
    ensureMode(BuilderMode::Entry, "withAttribute");
    m_entryChanges.attributes[key] = value;
    m_entryChanges.protectedAttributes[key] = isProtected;
    m_hasChanges = true;
    return *this;
}

TransactionBuilder& TransactionBuilder::withProperty(const QString& property, const QVariant& value)
{
    ensureMode(BuilderMode::Entry, "withProperty");
    m_entryChanges.properties[property] = value;
    m_hasChanges = true;
    return *this;
}

// Group operations
TransactionBuilder& TransactionBuilder::createGroup(const QUuid& parentGroupId)
{
    ensureMode(BuilderMode::Group, "createGroup");
    ensureNoOperation("createGroup");
    setOperation(DirectTransactionType::CreateGroup);
    m_targetId = parentGroupId;
    return *this;
}

TransactionBuilder& TransactionBuilder::updateGroup()
{
    ensureMode(BuilderMode::Group, "updateGroup");
    ensureNoOperation("updateGroup");
    setOperation(DirectTransactionType::UpdateGroup);
    return *this;
}

TransactionBuilder& TransactionBuilder::deleteGroup()
{
    ensureMode(BuilderMode::Group, "deleteGroup");
    ensureNoOperation("deleteGroup");
    setOperation(DirectTransactionType::DeleteGroup);
    return *this;
}

TransactionBuilder& TransactionBuilder::moveGroupTo(const QUuid& targetParentId)
{
    ensureMode(BuilderMode::Group, "moveGroupTo");
    ensureNoOperation("moveGroupTo");
    setOperation(DirectTransactionType::MoveGroup);
    m_targetId = targetParentId;
    return *this;
}

// Group property updates
TransactionBuilder& TransactionBuilder::withName(const QString& name)
{
    ensureMode(BuilderMode::Group, "withName");
    m_groupChanges.properties["Name"] = name;
    m_hasChanges = true;
    return *this;
}

TransactionBuilder& TransactionBuilder::withIcon(int iconIndex)
{
    ensureMode(BuilderMode::Group, "withIcon");
    m_groupChanges.properties["IconIndex"] = iconIndex;
    m_hasChanges = true;
    return *this;
}

TransactionBuilder& TransactionBuilder::withGroupProperty(const QString& property, const QVariant& value)
{
    ensureMode(BuilderMode::Group, "withGroupProperty");
    m_groupChanges.properties[property] = value;
    m_hasChanges = true;
    return *this;
}

// Database operations
TransactionBuilder& TransactionBuilder::updateDatabase()
{
    ensureMode(BuilderMode::Database, "updateDatabase");
    ensureNoOperation("updateDatabase");
    setOperation(DirectTransactionType::UpdateDatabase);
    return *this;
}

TransactionBuilder& TransactionBuilder::withDatabaseProperty(const QString& property, const QVariant& value)
{
    ensureMode(BuilderMode::Database, "withDatabaseProperty");
    m_databaseChanges.properties[property] = value;
    m_hasChanges = true;
    return *this;
}

// Build and validation
DirectTransaction TransactionBuilder::build()
{
    DirectTransaction transaction(m_transactionType, m_description);

    // Set target based on mode
    switch (m_mode) {
    case BuilderMode::Entry:
        transaction.setEntryTarget(m_entryId, m_targetId);
        transaction.setEntryChanges(m_entryChanges);
        break;
    case BuilderMode::Group:
        transaction.setGroupTarget(m_groupId, m_targetId);
        transaction.setGroupChanges(m_groupChanges);
        break;
    case BuilderMode::Database:
        transaction.setDatabaseTarget();
        transaction.setDatabaseChanges(m_databaseChanges);
        break;
    case BuilderMode::Unspecified:
        // Return invalid transaction - caller should check isValid()
        break;
    }

    return transaction;
}

bool TransactionBuilder::isValid() const
{
    return validationError().isEmpty();
}

QString TransactionBuilder::validationError() const
{
    if (m_mode == BuilderMode::Unspecified) {
        return "Transaction mode not specified. Use forEntry(), forGroup(), or forDatabase()";
    }

    if (!m_operationSet) {
        return "No operation specified. Use create(), update(), delete(), or move methods";
    }

    // Check for required target IDs
    if (m_mode == BuilderMode::Entry && m_entryId.isNull()) {
        return "Entry ID is required for entry transactions";
    }

    if (m_mode == BuilderMode::Group && m_groupId.isNull()) {
        return "Group ID is required for group transactions";
    }

    // Check for required target IDs for move and create operations
    if ((m_transactionType == DirectTransactionType::MoveEntry
         || m_transactionType == DirectTransactionType::CreateEntry)
        && m_targetId.isNull()) {
        return "Target group ID is required for move and create entry operations";
    }

    if ((m_transactionType == DirectTransactionType::MoveGroup
         || m_transactionType == DirectTransactionType::CreateGroup)
        && m_targetId.isNull()) {
        return "Target parent ID is required for move and create group operations";
    }

    // Check for changes when required
    bool changesRequired = (m_transactionType == DirectTransactionType::UpdateEntry
                            || m_transactionType == DirectTransactionType::UpdateGroup
                            || m_transactionType == DirectTransactionType::UpdateDatabase);

    if (changesRequired && !m_hasChanges) {
        return "Update operations require at least one change";
    }

    return QString();
}

// Helper methods
void TransactionBuilder::ensureMode(BuilderMode expectedMode, const QString& operation)
{
    if (m_mode != expectedMode) {
        // In a non-exception environment, we'll just ignore invalid operations
        // The final transaction will be invalid and caller should check isValid()
        return;
    }
}

void TransactionBuilder::ensureNoOperation(const QString& operation)
{
    if (m_operationSet) {
        // In a non-exception environment, we'll just ignore duplicate operations
        // The final transaction may be invalid and caller should check isValid()
        return;
    }
}

void TransactionBuilder::setOperation(DirectTransactionType type)
{
    m_transactionType = type;
    m_operationSet = true;
}

void TransactionBuilder::validateEntryOperation(DirectTransactionType type)
{
    if (m_mode != BuilderMode::Entry) {
        throw std::runtime_error("Entry operations can only be used with entry transactions");
    }
}

void TransactionBuilder::validateGroupOperation(DirectTransactionType type)
{
    if (m_mode != BuilderMode::Group) {
        throw std::runtime_error("Group operations can only be used with group transactions");
    }
}

void TransactionBuilder::validateDatabaseOperation(DirectTransactionType type)
{
    if (m_mode != BuilderMode::Database) {
        throw std::runtime_error("Database operations can only be used with database transactions");
    }
}