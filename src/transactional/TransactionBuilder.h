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

#ifndef KEEPASSX_TRANSACTIONBUILDER_H
#define KEEPASSX_TRANSACTIONBUILDER_H

#include "DirectTransaction.h"
#include <QString>
#include <QUuid>
#include <QVariant>

enum class BuilderMode
{
    Unspecified,
    Entry,
    Group,
    Database
};

class TransactionBuilder
{
public:
    TransactionBuilder();

    // Static factory methods for different transaction targets
    static TransactionBuilder forEntry(const QUuid& entryId);
    static TransactionBuilder forGroup(const QUuid& groupId);
    static TransactionBuilder forDatabase();

    // Description and metadata
    TransactionBuilder& withDescription(const QString& description);

    // Entry-specific operations
    TransactionBuilder& createEntry(const QUuid& parentGroupId);
    TransactionBuilder& updateEntry();
    TransactionBuilder& deleteEntry();
    TransactionBuilder& moveEntryTo(const QUuid& targetGroupId);

    // Entry attribute updates
    TransactionBuilder& withTitle(const QString& title);
    TransactionBuilder& withUsername(const QString& username);
    TransactionBuilder& withPassword(const QString& password);
    TransactionBuilder& withUrl(const QString& url);
    TransactionBuilder& withNotes(const QString& notes);
    TransactionBuilder& withAttribute(const QString& key, const QString& value, bool isProtected = false);
    TransactionBuilder& withProperty(const QString& property, const QVariant& value);

    // Group-specific operations
    TransactionBuilder& createGroup(const QUuid& parentGroupId);
    TransactionBuilder& updateGroup();
    TransactionBuilder& deleteGroup();
    TransactionBuilder& moveGroupTo(const QUuid& targetParentId);

    // Group property updates
    TransactionBuilder& withName(const QString& name);
    TransactionBuilder& withIcon(int iconIndex);
    TransactionBuilder& withGroupProperty(const QString& property, const QVariant& value);

    // Database-specific operations
    TransactionBuilder& updateDatabase();
    TransactionBuilder& withDatabaseProperty(const QString& property, const QVariant& value);

    // Build and validation
    DirectTransaction build();
    bool isValid() const;
    QString validationError() const;

private:
    BuilderMode m_mode;
    DirectTransactionType m_transactionType;
    QString m_description;

    // Target information
    QUuid m_entryId;
    QUuid m_groupId;
    QUuid m_targetId; // For move operations

    // Change data
    EntryChanges m_entryChanges;
    GroupChanges m_groupChanges;
    DatabaseChanges m_databaseChanges;

    // Internal state tracking
    bool m_operationSet;
    bool m_hasChanges;

    // Helper methods
    void ensureMode(BuilderMode expectedMode, const QString& operation);
    void ensureNoOperation(const QString& operation);
    void setOperation(DirectTransactionType type);
    void validateEntryOperation(DirectTransactionType type);
    void validateGroupOperation(DirectTransactionType type);
    void validateDatabaseOperation(DirectTransactionType type);
};

#endif // KEEPASSX_TRANSACTIONBUILDER_H