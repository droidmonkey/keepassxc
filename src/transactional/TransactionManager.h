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

#ifndef KEEPASSX_TRANSACTIONMANAGER_H
#define KEEPASSX_TRANSACTIONMANAGER_H

#include <QJsonObject>
#include <QObject>
#include <QSharedPointer>
#include <QStack>
#include <QVector>

#include "Transaction.h"

class Database;

class TransactionManager : public QObject
{
    Q_OBJECT

public:
    explicit TransactionManager(Database* database, QObject* parent = nullptr);

    // Transaction execution
    bool executeTransaction(const Transaction& transaction);
    bool executeTransactions(const QVector<Transaction>& transactions);

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
    QVector<Transaction> getHistory(int maxCount = -1) const;
    Transaction getLastTransaction() const;

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
    QVector<Transaction> getAuditTrail() const
    {
        return m_auditTrail;
    }

signals:
    void transactionExecuted(const Transaction& transaction);
    void transactionUndone(const Transaction& transaction);
    void transactionRedone(const Transaction& transaction);
    void historyChanged();
    void batchStarted(const QString& description);
    void batchEnded(const QString& description);

private:
    Database* m_database;
    QStack<Transaction> m_undoStack;
    QStack<Transaction> m_redoStack;
    QVector<Transaction> m_auditTrail;

    int m_maxHistorySize;
    bool m_auditEnabled;

    // Batch operation support
    int m_batchDepth;
    QVector<Transaction> m_batchTransactions;
    QString m_batchDescription;

    // Transaction execution helpers
    bool applyTransaction(const Transaction& transaction);
    Transaction createReverseTransaction(const Transaction& transaction);
    void recordTransaction(const Transaction& transaction);
    void trimHistory();

    // Individual transaction handlers
    bool handleCreateEntry(const Transaction& transaction);
    bool handleUpdateEntry(const Transaction& transaction);
    bool handleDeleteEntry(const Transaction& transaction);
    bool handleMoveEntry(const Transaction& transaction);
    bool handleCreateGroup(const Transaction& transaction);
    bool handleUpdateGroup(const Transaction& transaction);
    bool handleDeleteGroup(const Transaction& transaction);
    bool handleMoveGroup(const Transaction& transaction);
    bool handleUpdateDatabase(const Transaction& transaction);
};

#endif // KEEPASSX_TRANSACTIONMANAGER_H