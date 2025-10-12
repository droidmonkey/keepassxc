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

#include "TransactionManager.h"
#include "Database.h"
#include "Entry.h"
#include "Group.h"

TransactionManager::TransactionManager(Database* database, QObject* parent)
    : QObject(parent)
    , m_database(database)
    , m_maxHistorySize(100)
    , m_auditEnabled(true)
    , m_batchDepth(0)
{
}

bool TransactionManager::executeTransaction(const Transaction& transaction)
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

    // Clear redo stack since we're making a new change
    m_redoStack.clear();

    emit transactionExecuted(transaction);
    emit historyChanged();

    return true;
}

bool TransactionManager::executeTransactions(const QVector<Transaction>& transactions)
{
    beginBatch("Batch operation");

    for (const auto& transaction : transactions) {
        if (!executeTransaction(transaction)) {
            cancelBatch();
            return false;
        }
    }

    endBatch();
    return true;
}

bool TransactionManager::canUndo() const
{
    return !m_undoStack.isEmpty();
}

bool TransactionManager::canRedo() const
{
    return !m_redoStack.isEmpty();
}

bool TransactionManager::undo()
{
    if (!canUndo()) {
        return false;
    }

    Transaction transaction = m_undoStack.pop();
    Transaction reverseTransaction = createReverseTransaction(transaction);

    if (!applyTransaction(reverseTransaction)) {
        // Restore the transaction to the undo stack
        m_undoStack.push(transaction);
        return false;
    }

    m_redoStack.push(transaction);

    emit transactionUndone(transaction);
    emit historyChanged();

    return true;
}

bool TransactionManager::redo()
{
    if (!canRedo()) {
        return false;
    }

    Transaction transaction = m_redoStack.pop();

    if (!applyTransaction(transaction)) {
        // Restore the transaction to the redo stack
        m_redoStack.push(transaction);
        return false;
    }

    m_undoStack.push(transaction);

    emit transactionRedone(transaction);
    emit historyChanged();

    return true;
}

void TransactionManager::clearHistory()
{
    m_undoStack.clear();
    m_redoStack.clear();
    emit historyChanged();
}

void TransactionManager::setMaxHistorySize(int size)
{
    m_maxHistorySize = qMax(0, size);
    trimHistory();
}

QVector<Transaction> TransactionManager::getHistory(int maxCount) const
{
    QVector<Transaction> history;
    const auto& stack = m_undoStack;

    int count = (maxCount > 0) ? qMin(maxCount, stack.size()) : stack.size();

    for (int i = stack.size() - count; i < stack.size(); ++i) {
        history.append(stack[i]);
    }

    return history;
}

Transaction TransactionManager::getLastTransaction() const
{
    if (m_undoStack.isEmpty()) {
        return Transaction();
    }
    return m_undoStack.top();
}

void TransactionManager::beginBatch(const QString& description)
{
    if (m_batchDepth == 0) {
        m_batchTransactions.clear();
        m_batchDescription = description.isEmpty() ? "Batch operation" : description;
        emit batchStarted(m_batchDescription);
    }
    ++m_batchDepth;
}

void TransactionManager::endBatch()
{
    if (m_batchDepth == 0) {
        return;
    }

    --m_batchDepth;

    if (m_batchDepth == 0) {
        // Execute all batched transactions as a single undoable operation
        if (!m_batchTransactions.isEmpty()) {
            // Create a compound transaction
            Transaction batchTransaction(TransactionType::UpdateDatabase, m_batchDescription);

            // For now, we'll execute each transaction individually
            // In a full implementation, we'd want to optimize this
            for (const auto& transaction : m_batchTransactions) {
                applyTransaction(transaction);
            }

            recordTransaction(batchTransaction);
            m_redoStack.clear();

            emit transactionExecuted(batchTransaction);
            emit historyChanged();
        }

        emit batchEnded(m_batchDescription);
        m_batchTransactions.clear();
        m_batchDescription.clear();
    }
}

void TransactionManager::cancelBatch()
{
    if (m_batchDepth > 0) {
        m_batchDepth = 0;
        m_batchTransactions.clear();
        m_batchDescription.clear();
    }
}

bool TransactionManager::applyTransaction(const Transaction& transaction)
{
    switch (transaction.type()) {
    case TransactionType::CreateEntry:
        return handleCreateEntry(transaction);
    case TransactionType::UpdateEntry:
        return handleUpdateEntry(transaction);
    case TransactionType::DeleteEntry:
        return handleDeleteEntry(transaction);
    case TransactionType::MoveEntry:
        return handleMoveEntry(transaction);
    case TransactionType::CreateGroup:
        return handleCreateGroup(transaction);
    case TransactionType::UpdateGroup:
        return handleUpdateGroup(transaction);
    case TransactionType::DeleteGroup:
        return handleDeleteGroup(transaction);
    case TransactionType::MoveGroup:
        return handleMoveGroup(transaction);
    case TransactionType::UpdateDatabase:
        return handleUpdateDatabase(transaction);
    default:
        return false;
    }
}

Transaction TransactionManager::createReverseTransaction(const Transaction& transaction)
{
    // This is a simplified implementation
    // In a full implementation, we'd need to capture the previous state
    Transaction reverseTransaction;
    reverseTransaction.setTarget("entryId", transaction.target("entryId"));

    // For now, return a placeholder - full implementation would restore previous values
    return reverseTransaction;
}

void TransactionManager::recordTransaction(const Transaction& transaction)
{
    m_undoStack.push(transaction);

    if (m_auditEnabled) {
        m_auditTrail.append(transaction);
    }

    trimHistory();
}

void TransactionManager::trimHistory()
{
    while (m_undoStack.size() > m_maxHistorySize) {
        m_undoStack.removeFirst();
    }
}

bool TransactionManager::handleCreateEntry(const Transaction& transaction)
{
    Q_UNUSED(transaction)
    // Placeholder - would create new entry in database
    return true;
}

bool TransactionManager::handleUpdateEntry(const Transaction& transaction)
{
    Q_UNUSED(transaction)
    // Placeholder - would update entry in database
    return true;
}

bool TransactionManager::handleDeleteEntry(const Transaction& transaction)
{
    Q_UNUSED(transaction)
    // Placeholder - would delete entry from database
    return true;
}

bool TransactionManager::handleMoveEntry(const Transaction& transaction)
{
    Q_UNUSED(transaction)
    // Placeholder - would move entry to different group
    return true;
}

bool TransactionManager::handleCreateGroup(const Transaction& transaction)
{
    Q_UNUSED(transaction)
    // Placeholder - would create new group in database
    return true;
}

bool TransactionManager::handleUpdateGroup(const Transaction& transaction)
{
    Q_UNUSED(transaction)
    // Placeholder - would update group in database
    return true;
}

bool TransactionManager::handleDeleteGroup(const Transaction& transaction)
{
    Q_UNUSED(transaction)
    // Placeholder - would delete group from database
    return true;
}

bool TransactionManager::handleMoveGroup(const Transaction& transaction)
{
    Q_UNUSED(transaction)
    // Placeholder - would move group to different parent
    return true;
}

bool TransactionManager::handleUpdateDatabase(const Transaction& transaction)
{
    Q_UNUSED(transaction)
    // Placeholder - would update database metadata
    return true;
}