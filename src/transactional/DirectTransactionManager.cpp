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

#include "DirectTransactionManager.h"
#include "Database.h"
#include "Entry.h"
#include "Group.h"

DirectTransactionManager::DirectTransactionManager(Database* database, QObject* parent)
    : QObject(parent)
    , m_database(database)
    , m_maxHistorySize(100)
    , m_auditEnabled(true)
    , m_batchDepth(0)
{
}

bool DirectTransactionManager::executeTransaction(const DirectTransaction& transaction)
{
    if (!transaction.isValid()) {
        return false;
    }

    if (m_batchDepth > 0) {
        // In batch mode, collect transactions
        m_batchTransactions.append(transaction);
        return true;
    }

    // Execute the transaction
    if (!applyTransaction(transaction)) {
        return false;
    }

    // Record for undo/redo
    recordTransaction(transaction);

    // Emit signal
    emit transactionExecuted(transaction);

    return true;
}

bool DirectTransactionManager::executeTransactions(const QVector<DirectTransaction>& transactions)
{
    beginBatch("Multiple transactions");
    bool success = true;

    for (const auto& transaction : transactions) {
        if (!executeTransaction(transaction)) {
            success = false;
            break;
        }
    }

    if (success) {
        endBatch();
    } else {
        cancelBatch();
    }

    return success;
}

bool DirectTransactionManager::canUndo() const
{
    return !m_undoStack.isEmpty();
}

bool DirectTransactionManager::canRedo() const
{
    return !m_redoStack.isEmpty();
}

bool DirectTransactionManager::undo()
{
    if (!canUndo()) {
        return false;
    }

    DirectTransaction transaction = m_undoStack.pop();
    DirectTransaction reverseTransaction = createReverseTransaction(transaction);

    if (!applyTransaction(reverseTransaction)) {
        // Push back if undo failed
        m_undoStack.push(transaction);
        return false;
    }

    m_redoStack.push(transaction);
    emit transactionUndone(transaction);
    emit historyChanged();

    return true;
}

bool DirectTransactionManager::redo()
{
    if (!canRedo()) {
        return false;
    }

    DirectTransaction transaction = m_redoStack.pop();

    if (!applyTransaction(transaction)) {
        // Push back if redo failed
        m_redoStack.push(transaction);
        return false;
    }

    m_undoStack.push(transaction);
    emit transactionRedone(transaction);
    emit historyChanged();

    return true;
}

void DirectTransactionManager::clearHistory()
{
    m_undoStack.clear();
    m_redoStack.clear();
    emit historyChanged();
}

void DirectTransactionManager::setMaxHistorySize(int size)
{
    m_maxHistorySize = size;
    trimHistory();
}

QVector<DirectTransaction> DirectTransactionManager::getHistory(int maxCount) const
{
    QVector<DirectTransaction> history;
    QStack<DirectTransaction> temp = m_undoStack;

    while (!temp.isEmpty() && (maxCount < 0 || history.size() < maxCount)) {
        history.prepend(temp.pop());
    }

    return history;
}

DirectTransaction DirectTransactionManager::getLastTransaction() const
{
    if (m_undoStack.isEmpty()) {
        return DirectTransaction();
    }
    return m_undoStack.top();
}

void DirectTransactionManager::beginBatch(const QString& description)
{
    if (m_batchDepth == 0) {
        m_batchTransactions.clear();
        m_batchDescription = description.isEmpty() ? "Batch operation" : description;
        emit batchStarted(m_batchDescription);
    }
    m_batchDepth++;
}

void DirectTransactionManager::endBatch()
{
    if (m_batchDepth <= 0) {
        return;
    }

    m_batchDepth--;
    if (m_batchDepth == 0) {
        // Execute all batched transactions as a single unit
        if (!m_batchTransactions.isEmpty()) {
            // Create a composite transaction or execute them all
            for (const auto& transaction : m_batchTransactions) {
                applyTransaction(transaction);
                recordTransaction(transaction);
            }
        }
        emit batchEnded(m_batchDescription);
        m_batchTransactions.clear();
    }
}

void DirectTransactionManager::cancelBatch()
{
    if (m_batchDepth > 0) {
        m_batchDepth = 0;
        m_batchTransactions.clear();
        emit batchEnded(m_batchDescription + " (cancelled)");
    }
}

bool DirectTransactionManager::applyTransaction(const DirectTransaction& transaction)
{
    switch (transaction.type()) {
    case DirectTransactionType::CreateEntry:
        return handleCreateEntry(transaction);
    case DirectTransactionType::UpdateEntry:
        return handleUpdateEntry(transaction);
    case DirectTransactionType::DeleteEntry:
        return handleDeleteEntry(transaction);
    case DirectTransactionType::MoveEntry:
        return handleMoveEntry(transaction);
    case DirectTransactionType::CreateGroup:
        return handleCreateGroup(transaction);
    case DirectTransactionType::UpdateGroup:
        return handleUpdateGroup(transaction);
    case DirectTransactionType::DeleteGroup:
        return handleDeleteGroup(transaction);
    case DirectTransactionType::MoveGroup:
        return handleMoveGroup(transaction);
    case DirectTransactionType::UpdateDatabase:
        return handleUpdateDatabase(transaction);
    }
    return false;
}

DirectTransaction DirectTransactionManager::createReverseTransaction(const DirectTransaction& transaction)
{
    DirectTransaction reverse;

    // Set reverse type
    switch (transaction.type()) {
    case DirectTransactionType::CreateEntry:
        reverse = DirectTransaction(DirectTransactionType::DeleteEntry, "Undo: " + transaction.description());
        reverse.setEntryTarget(transaction.entryTarget().entryId);
        break;
    case DirectTransactionType::UpdateEntry:
        reverse = DirectTransaction(DirectTransactionType::UpdateEntry, "Undo: " + transaction.description());
        reverse.setEntryTarget(transaction.entryTarget().entryId);
        reverse.setEntryChanges(getPreviousEntryState(transaction.entryTarget().entryId));
        break;
    case DirectTransactionType::DeleteEntry:
        reverse = DirectTransaction(DirectTransactionType::CreateEntry, "Undo: " + transaction.description());
        reverse.setEntryTarget(transaction.entryTarget().entryId, transaction.entryTarget().groupId);
        reverse.setEntryChanges(getPreviousEntryState(transaction.entryTarget().entryId));
        break;
    case DirectTransactionType::MoveEntry:
        reverse = DirectTransaction(DirectTransactionType::MoveEntry, "Undo: " + transaction.description());
        reverse.setEntryTarget(
            transaction.entryTarget().entryId,
            getPreviousEntryState(transaction.entryTarget().entryId).properties.value("groupId").toUuid());
        break;
    case DirectTransactionType::CreateGroup:
        reverse = DirectTransaction(DirectTransactionType::DeleteGroup, "Undo: " + transaction.description());
        reverse.setGroupTarget(transaction.groupTarget().groupId);
        break;
    case DirectTransactionType::UpdateGroup:
        reverse = DirectTransaction(DirectTransactionType::UpdateGroup, "Undo: " + transaction.description());
        reverse.setGroupTarget(transaction.groupTarget().groupId);
        reverse.setGroupChanges(getPreviousGroupState(transaction.groupTarget().groupId));
        break;
    case DirectTransactionType::DeleteGroup:
        reverse = DirectTransaction(DirectTransactionType::CreateGroup, "Undo: " + transaction.description());
        reverse.setGroupTarget(transaction.groupTarget().groupId, transaction.groupTarget().parentId);
        reverse.setGroupChanges(getPreviousGroupState(transaction.groupTarget().groupId));
        break;
    case DirectTransactionType::MoveGroup:
        reverse = DirectTransaction(DirectTransactionType::MoveGroup, "Undo: " + transaction.description());
        reverse.setGroupTarget(
            transaction.groupTarget().groupId,
            getPreviousGroupState(transaction.groupTarget().groupId).properties.value("parentId").toUuid());
        break;
    case DirectTransactionType::UpdateDatabase:
        reverse = DirectTransaction(DirectTransactionType::UpdateDatabase, "Undo: " + transaction.description());
        reverse.setDatabaseTarget();
        reverse.setDatabaseChanges(getPreviousDatabaseState());
        break;
    }

    return reverse;
}

void DirectTransactionManager::recordTransaction(const DirectTransaction& transaction)
{
    // Clear redo stack when new transaction is executed
    m_redoStack.clear();

    // Add to undo stack
    m_undoStack.push(transaction);

    // Add to audit trail if enabled
    if (m_auditEnabled) {
        m_auditTrail.append(transaction);
    }

    // Trim history if needed
    trimHistory();

    emit historyChanged();
}

void DirectTransactionManager::trimHistory()
{
    while (m_undoStack.size() > m_maxHistorySize) {
        m_undoStack.removeFirst();
    }
}

// Convenience methods implementation
bool DirectTransactionManager::createEntry(const QUuid& groupId,
                                           const QString& title,
                                           const QString& username,
                                           const QString& password)
{
    DirectTransaction transaction(DirectTransactionType::CreateEntry, "Create entry");
    transaction.setEntryTarget(QUuid::createUuid(), groupId);

    if (!title.isEmpty()) {
        transaction.setEntryAttribute("Title", title);
    }
    if (!username.isEmpty()) {
        transaction.setEntryAttribute("UserName", username);
    }
    if (!password.isEmpty()) {
        transaction.setEntryAttribute("Password", password, true);
    }

    return executeTransaction(transaction);
}

bool DirectTransactionManager::updateEntryTitle(const QUuid& entryId, const QString& title)
{
    DirectTransaction transaction(DirectTransactionType::UpdateEntry, "Update entry title");
    transaction.setEntryTarget(entryId);
    transaction.setEntryAttribute("Title", title);
    return executeTransaction(transaction);
}

bool DirectTransactionManager::updateEntryPassword(const QUuid& entryId, const QString& password)
{
    DirectTransaction transaction(DirectTransactionType::UpdateEntry, "Update entry password");
    transaction.setEntryTarget(entryId);
    transaction.setEntryAttribute("Password", password, true);
    return executeTransaction(transaction);
}

bool DirectTransactionManager::updateEntryAttribute(const QUuid& entryId,
                                                    const QString& key,
                                                    const QString& value,
                                                    bool isProtected)
{
    DirectTransaction transaction(DirectTransactionType::UpdateEntry, "Update entry attribute");
    transaction.setEntryTarget(entryId);
    transaction.setEntryAttribute(key, value, isProtected);
    return executeTransaction(transaction);
}

bool DirectTransactionManager::deleteEntry(const QUuid& entryId)
{
    DirectTransaction transaction(DirectTransactionType::DeleteEntry, "Delete entry");
    transaction.setEntryTarget(entryId);
    return executeTransaction(transaction);
}

bool DirectTransactionManager::moveEntry(const QUuid& entryId, const QUuid& newGroupId)
{
    DirectTransaction transaction(DirectTransactionType::MoveEntry, "Move entry");
    transaction.setEntryTarget(entryId, newGroupId);
    return executeTransaction(transaction);
}

bool DirectTransactionManager::createGroup(const QUuid& parentId, const QString& name)
{
    DirectTransaction transaction(DirectTransactionType::CreateGroup, "Create group");
    transaction.setGroupTarget(QUuid::createUuid(), parentId);
    transaction.setGroupProperty("name", name);
    return executeTransaction(transaction);
}

bool DirectTransactionManager::updateGroupName(const QUuid& groupId, const QString& name)
{
    DirectTransaction transaction(DirectTransactionType::UpdateGroup, "Update group name");
    transaction.setGroupTarget(groupId);
    transaction.setGroupProperty("name", name);
    return executeTransaction(transaction);
}

bool DirectTransactionManager::deleteGroup(const QUuid& groupId)
{
    DirectTransaction transaction(DirectTransactionType::DeleteGroup, "Delete group");
    transaction.setGroupTarget(groupId);
    return executeTransaction(transaction);
}

bool DirectTransactionManager::moveGroup(const QUuid& groupId, const QUuid& newParentId)
{
    DirectTransaction transaction(DirectTransactionType::MoveGroup, "Move group");
    transaction.setGroupTarget(groupId, newParentId);
    return executeTransaction(transaction);
}

// Transaction handler implementations (simplified for now)
bool DirectTransactionManager::handleCreateEntry(const DirectTransaction& transaction)
{
    // In a real implementation, this would interact with the actual database structures
    // For now, return true to indicate successful handling
    return true;
}

bool DirectTransactionManager::handleUpdateEntry(const DirectTransaction& transaction)
{
    return true;
}

bool DirectTransactionManager::handleDeleteEntry(const DirectTransaction& transaction)
{
    return true;
}

bool DirectTransactionManager::handleMoveEntry(const DirectTransaction& transaction)
{
    return true;
}

bool DirectTransactionManager::handleCreateGroup(const DirectTransaction& transaction)
{
    return true;
}

bool DirectTransactionManager::handleUpdateGroup(const DirectTransaction& transaction)
{
    return true;
}

bool DirectTransactionManager::handleDeleteGroup(const DirectTransaction& transaction)
{
    return true;
}

bool DirectTransactionManager::handleMoveGroup(const DirectTransaction& transaction)
{
    return true;
}

bool DirectTransactionManager::handleUpdateDatabase(const DirectTransaction& transaction)
{
    return true;
}

// Helper methods to get previous states (simplified for now)
EntryChanges DirectTransactionManager::getPreviousEntryState(const QUuid& entryId) const
{
    Q_UNUSED(entryId);
    // In a real implementation, this would query the actual entry state
    EntryChanges changes;
    return changes;
}

GroupChanges DirectTransactionManager::getPreviousGroupState(const QUuid& groupId) const
{
    Q_UNUSED(groupId);
    // In a real implementation, this would query the actual group state
    GroupChanges changes;
    return changes;
}

DatabaseChanges DirectTransactionManager::getPreviousDatabaseState() const
{
    // In a real implementation, this would query the actual database state
    DatabaseChanges changes;
    return changes;
}