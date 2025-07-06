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

#ifndef KEEPASSX_DIRECTTRANSACTIONMANAGER_H
#define KEEPASSX_DIRECTTRANSACTIONMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include <QStack>
#include <QVector>

#include "DirectTransaction.h"

class Database;

class DirectTransactionManager : public QObject
{
    Q_OBJECT

public:
    explicit DirectTransactionManager(Database* database, QObject* parent = nullptr);

    // Transaction execution
    bool executeTransaction(const DirectTransaction& transaction);
    bool executeTransactions(const QVector<DirectTransaction>& transactions);

    // Undo/Redo functionality
    bool canUndo() const;
    bool canRedo() const;
    bool undo();
    bool redo();
    void clearHistory();

    // History management
    int maxHistorySize() const
    {
        return m_maxHistorySize;
    }
    void setMaxHistorySize(int size);
    int historySize() const
    {
        return m_undoStack.size();
    }

    // History access
    QVector<DirectTransaction> getHistory(int maxCount = -1) const;
    DirectTransaction getLastTransaction() const;

    // Batch operations
    void beginBatch(const QString& description = QString());
    void endBatch();
    void cancelBatch();
    bool isInBatch() const
    {
        return m_batchDepth > 0;
    }

    // Audit trail
    void enableAuditTrail(bool enable)
    {
        m_auditEnabled = enable;
    }
    bool isAuditTrailEnabled() const
    {
        return m_auditEnabled;
    }
    QVector<DirectTransaction> getAuditTrail() const
    {
        return m_auditTrail;
    }

    // Convenience methods for common operations
    bool createEntry(const QUuid& groupId,
                     const QString& title = QString(),
                     const QString& username = QString(),
                     const QString& password = QString());
    bool updateEntryTitle(const QUuid& entryId, const QString& title);
    bool updateEntryPassword(const QUuid& entryId, const QString& password);
    bool updateEntryAttribute(const QUuid& entryId, const QString& key, const QString& value, bool isProtected = false);
    bool deleteEntry(const QUuid& entryId);
    bool moveEntry(const QUuid& entryId, const QUuid& newGroupId);

    bool createGroup(const QUuid& parentId, const QString& name);
    bool updateGroupName(const QUuid& groupId, const QString& name);
    bool deleteGroup(const QUuid& groupId);
    bool moveGroup(const QUuid& groupId, const QUuid& newParentId);

signals:
    void transactionExecuted(const DirectTransaction& transaction);
    void transactionUndone(const DirectTransaction& transaction);
    void transactionRedone(const DirectTransaction& transaction);
    void historyChanged();
    void batchStarted(const QString& description);
    void batchEnded(const QString& description);

private:
    Database* m_database;
    QStack<DirectTransaction> m_undoStack;
    QStack<DirectTransaction> m_redoStack;
    QVector<DirectTransaction> m_auditTrail;

    int m_maxHistorySize;
    bool m_auditEnabled;

    // Batch operation support
    int m_batchDepth;
    QVector<DirectTransaction> m_batchTransactions;
    QString m_batchDescription;

    // Transaction execution helpers
    bool applyTransaction(const DirectTransaction& transaction);
    DirectTransaction createReverseTransaction(const DirectTransaction& transaction);
    void recordTransaction(const DirectTransaction& transaction);
    void trimHistory();

    // Individual transaction handlers
    bool handleCreateEntry(const DirectTransaction& transaction);
    bool handleUpdateEntry(const DirectTransaction& transaction);
    bool handleDeleteEntry(const DirectTransaction& transaction);
    bool handleMoveEntry(const DirectTransaction& transaction);
    bool handleCreateGroup(const DirectTransaction& transaction);
    bool handleUpdateGroup(const DirectTransaction& transaction);
    bool handleDeleteGroup(const DirectTransaction& transaction);
    bool handleMoveGroup(const DirectTransaction& transaction);
    bool handleUpdateDatabase(const DirectTransaction& transaction);

    // Helper methods for getting previous values for undo
    EntryChanges getPreviousEntryState(const QUuid& entryId) const;
    GroupChanges getPreviousGroupState(const QUuid& groupId) const;
    DatabaseChanges getPreviousDatabaseState() const;
};

#endif // KEEPASSX_DIRECTTRANSACTIONMANAGER_H