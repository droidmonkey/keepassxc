# Technical Specifications for Transactional Edits

## Detailed API Design

### Core Transaction Interface

```cpp
// src/core/transactions/Transaction.h
#ifndef KEEPASSXC_TRANSACTION_H
#define KEEPASSXC_TRANSACTION_H

#include <QDateTime>
#include <QJsonObject>
#include <QUuid>

namespace Transactions {

enum class TransactionType {
    CreateEntry,
    UpdateEntry,
    DeleteEntry,
    MoveEntry,
    CreateGroup,
    UpdateGroup,
    DeleteGroup,
    MoveGroup,
    UpdateMetadata,
    BulkOperation,
    CreateAttachment,
    UpdateAttachment,
    DeleteAttachment
};

class Transaction {
public:
    Transaction(TransactionType type, const QJsonObject& data, const QString& description = QString());
    
    // Core properties
    QUuid id() const { return m_id; }
    TransactionType type() const { return m_type; }
    QDateTime timestamp() const { return m_timestamp; }
    QString description() const { return m_description; }
    QJsonObject data() const { return m_data; }
    
    // Serialization
    QJsonObject toJson() const;
    static Transaction fromJson(const QJsonObject& json);
    
    // Validation
    bool isValid() const;
    QString validationError() const;
    
    // Metadata
    void setMetadata(const QString& key, const QVariant& value);
    QVariant metadata(const QString& key) const;
    
    // Factory methods for common operations
    static Transaction createEntry(const QUuid& groupId, const QString& title, 
                                   const QMap<QString, QString>& attributes = {});
    static Transaction updateEntryAttribute(const QUuid& entryId, const QString& key, 
                                            const QString& value, bool isProtected = false);
    static Transaction deleteEntry(const QUuid& entryId);
    static Transaction moveEntry(const QUuid& entryId, const QUuid& newGroupId);
    static Transaction createGroup(const QUuid& parentId, const QString& name);
    static Transaction updateGroup(const QUuid& groupId, const QJsonObject& changes);
    static Transaction deleteGroup(const QUuid& groupId);

private:
    QUuid m_id;
    TransactionType m_type;
    QDateTime m_timestamp;
    QString m_description;
    QJsonObject m_data;
    QJsonObject m_metadata;
};

} // namespace Transactions

#endif // KEEPASSXC_TRANSACTION_H
```

### Transaction Processor Implementation

```cpp
// src/core/transactions/TransactionProcessor.h
#ifndef KEEPASSXC_TRANSACTIONPROCESSOR_H
#define KEEPASSXC_TRANSACTIONPROCESSOR_H

#include "Transaction.h"
#include "core/Database.h"
#include <QSharedPointer>

namespace Transactions {

struct TransactionResult {
    bool success = false;
    QString error;
    QSharedPointer<const Database> newState;
    QJsonObject changes; // What actually changed
};

class TransactionProcessor : public QObject {
    Q_OBJECT

public:
    explicit TransactionProcessor(QSharedPointer<const Database> initialState, QObject* parent = nullptr);
    
    // Core transaction processing
    TransactionResult apply(const Transaction& transaction);
    TransactionResult applyBatch(const QList<Transaction>& transactions);
    
    // State management
    QSharedPointer<const Database> currentState() const { return m_currentState; }
    void setState(QSharedPointer<const Database> state);
    
    // Transaction validation
    bool canApply(const Transaction& transaction) const;
    QString validateTransaction(const Transaction& transaction) const;
    
    // Preview functionality
    TransactionResult preview(const Transaction& transaction) const;
    
signals:
    void transactionApplied(const Transaction& transaction, const TransactionResult& result);
    void batchApplied(const QList<Transaction>& transactions, const TransactionResult& result);
    void stateChanged(QSharedPointer<const Database> newState);

private:
    QSharedPointer<const Database> m_currentState;
    
    // Individual transaction handlers
    TransactionResult handleCreateEntry(const Transaction& transaction) const;
    TransactionResult handleUpdateEntry(const Transaction& transaction) const;
    TransactionResult handleDeleteEntry(const Transaction& transaction) const;
    TransactionResult handleMoveEntry(const Transaction& transaction) const;
    TransactionResult handleCreateGroup(const Transaction& transaction) const;
    TransactionResult handleUpdateGroup(const Transaction& transaction) const;
    TransactionResult handleDeleteGroup(const Transaction& transaction) const;
    TransactionResult handleMoveGroup(const Transaction& transaction) const;
    TransactionResult handleUpdateMetadata(const Transaction& transaction) const;
    TransactionResult handleBulkOperation(const Transaction& transaction) const;
    
    // Validation helpers
    bool entryExists(const QUuid& entryId) const;
    bool groupExists(const QUuid& groupId) const;
    Entry* findEntry(const QUuid& entryId) const;
    Group* findGroup(const QUuid& groupId) const;
};

} // namespace Transactions

#endif // KEEPASSXC_TRANSACTIONPROCESSOR_H
```

### Immutable Database Design

```cpp
// src/core/ImmutableDatabase.h
#ifndef KEEPASSXC_IMMUTABLEDATABASE_H
#define KEEPASSXC_IMMUTABLEDATABASE_H

#include "Database.h"
#include "ImmutableGroup.h"
#include "ImmutableEntry.h"
#include <QSharedPointer>

class ImmutableDatabase {
public:
    // Construction from mutable database
    explicit ImmutableDatabase(const Database& mutableDb);
    
    // Copy constructor with copy-on-write
    ImmutableDatabase(const ImmutableDatabase& other) = default;
    
    // Core accessors (read-only)
    QSharedPointer<const ImmutableGroup> rootGroup() const { return m_rootGroup; }
    QSharedPointer<const Metadata> metadata() const { return m_metadata; }
    QUuid uuid() const { return m_uuid; }
    QString filePath() const { return m_filePath; }
    
    // Immutable modification methods (return new instances)
    ImmutableDatabase withRootGroup(QSharedPointer<const ImmutableGroup> newRoot) const;
    ImmutableDatabase withMetadata(QSharedPointer<const Metadata> newMetadata) const;
    ImmutableDatabase withFilePath(const QString& path) const;
    
    // Entry operations
    ImmutableDatabase withNewEntry(const QUuid& groupId, QSharedPointer<const ImmutableEntry> entry) const;
    ImmutableDatabase withUpdatedEntry(const QUuid& entryId, QSharedPointer<const ImmutableEntry> entry) const;
    ImmutableDatabase withoutEntry(const QUuid& entryId) const;
    ImmutableDatabase withMovedEntry(const QUuid& entryId, const QUuid& newGroupId) const;
    
    // Group operations
    ImmutableDatabase withNewGroup(const QUuid& parentId, QSharedPointer<const ImmutableGroup> group) const;
    ImmutableDatabase withUpdatedGroup(const QUuid& groupId, QSharedPointer<const ImmutableGroup> group) const;
    ImmutableDatabase withoutGroup(const QUuid& groupId) const;
    ImmutableDatabase withMovedGroup(const QUuid& groupId, const QUuid& newParentId) const;
    
    // Search and query
    QSharedPointer<const ImmutableEntry> findEntry(const QUuid& entryId) const;
    QSharedPointer<const ImmutableGroup> findGroup(const QUuid& groupId) const;
    QList<QSharedPointer<const ImmutableEntry>> allEntries() const;
    QList<QSharedPointer<const ImmutableGroup>> allGroups() const;
    
    // Conversion to mutable (for compatibility)
    QSharedPointer<Database> toMutableDatabase() const;
    
    // Comparison and hashing
    bool operator==(const ImmutableDatabase& other) const;
    bool operator!=(const ImmutableDatabase& other) const;
    QString hash() const; // For efficient comparison
    
private:
    QSharedPointer<const ImmutableGroup> m_rootGroup;
    QSharedPointer<const Metadata> m_metadata;
    QUuid m_uuid;
    QString m_filePath;
    
    // Helper methods for tree operations
    QSharedPointer<const ImmutableGroup> updateGroupInTree(
        QSharedPointer<const ImmutableGroup> root,
        const QUuid& targetId,
        QSharedPointer<const ImmutableGroup> newGroup) const;
    
    QSharedPointer<const ImmutableGroup> removeGroupFromTree(
        QSharedPointer<const ImmutableGroup> root,
        const QUuid& targetId) const;
};

#endif // KEEPASSXC_IMMUTABLEDATABASE_H
```

### Undo/Redo Manager Implementation

```cpp
// src/core/transactions/UndoRedoManager.h
#ifndef KEEPASSXC_UNDOREDOMANAGER_H
#define KEEPASSXC_UNDOREDOMANAGER_H

#include "Transaction.h"
#include "TransactionProcessor.h"
#include <QObject>
#include <QStack>

namespace Transactions {

class UndoRedoManager : public QObject {
    Q_OBJECT

public:
    explicit UndoRedoManager(QSharedPointer<TransactionProcessor> processor, QObject* parent = nullptr);
    
    // Transaction execution with undo support
    TransactionResult executeTransaction(const Transaction& transaction);
    TransactionResult executeBatch(const QList<Transaction>& transactions);
    
    // Undo/Redo operations
    bool canUndo() const;
    bool canRedo() const;
    TransactionResult undo();
    TransactionResult redo();
    
    // History management
    QList<Transaction> undoHistory(int maxCount = 50) const;
    QList<Transaction> redoHistory(int maxCount = 50) const;
    void clearHistory();
    void setMaxHistorySize(int size);
    
    // Transaction groups (for atomic operations)
    void beginTransactionGroup(const QString& description);
    void endTransactionGroup();
    void cancelTransactionGroup();
    
    // State inspection
    QString lastUndoDescription() const;
    QString lastRedoDescription() const;
    int undoCount() const;
    int redoCount() const;

signals:
    void transactionExecuted(const Transaction& transaction);
    void undoPerformed(const Transaction& transaction);
    void redoPerformed(const Transaction& transaction);
    void historyChanged();
    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);

private:
    QSharedPointer<TransactionProcessor> m_processor;
    QStack<Transaction> m_undoStack;
    QStack<Transaction> m_redoStack;
    int m_maxHistorySize = 100;
    
    // Transaction groups
    bool m_inTransactionGroup = false;
    QString m_groupDescription;
    QList<Transaction> m_currentGroup;
    
    // Helper methods
    Transaction createInverseTransaction(const Transaction& original) const;
    void addToUndoStack(const Transaction& transaction);
    void clearRedoStack();
    void pruneHistory();
};

} // namespace Transactions

#endif // KEEPASSXC_UNDOREDOMANAGER_H
```

## JSON Transaction Schema

### Base Transaction Schema

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "KeePassXC Transaction",
  "type": "object",
  "required": ["id", "type", "timestamp", "data"],
  "properties": {
    "id": {
      "type": "string",
      "format": "uuid",
      "description": "Unique identifier for the transaction"
    },
    "type": {
      "type": "string",
      "enum": [
        "CreateEntry", "UpdateEntry", "DeleteEntry", "MoveEntry",
        "CreateGroup", "UpdateGroup", "DeleteGroup", "MoveGroup",
        "UpdateMetadata", "BulkOperation",
        "CreateAttachment", "UpdateAttachment", "DeleteAttachment"
      ]
    },
    "timestamp": {
      "type": "string",
      "format": "date-time",
      "description": "ISO 8601 timestamp when transaction was created"
    },
    "description": {
      "type": "string",
      "description": "Human-readable description of the transaction"
    },
    "data": {
      "type": "object",
      "description": "Transaction-specific data"
    },
    "metadata": {
      "type": "object",
      "properties": {
        "userId": {"type": "string"},
        "applicationVersion": {"type": "string"},
        "platform": {"type": "string"},
        "source": {"type": "string"}
      }
    }
  }
}
```

### Entry Creation Transaction

```json
{
  "id": "550e8400-e29b-41d4-a716-446655440000",
  "type": "CreateEntry",
  "timestamp": "2024-01-15T10:30:00Z",
  "description": "Create new entry: GitHub Account",
  "data": {
    "groupId": "parent-group-uuid",
    "entry": {
      "uuid": "entry-uuid",
      "title": "GitHub Account",
      "iconNumber": 0,
      "attributes": {
        "UserName": {
          "value": "myusername",
          "protected": false
        },
        "Password": {
          "value": "mypassword123",
          "protected": true
        },
        "URL": {
          "value": "https://github.com",
          "protected": false
        },
        "Notes": {
          "value": "My GitHub account for personal projects",
          "protected": false
        }
      },
      "timeInfo": {
        "creationTime": "2024-01-15T10:30:00Z",
        "lastModificationTime": "2024-01-15T10:30:00Z",
        "lastAccessTime": "2024-01-15T10:30:00Z",
        "expiryTime": null,
        "expires": false
      },
      "attachments": {},
      "customData": {},
      "tags": ["work", "development"]
    }
  },
  "metadata": {
    "userId": "user@example.com",
    "applicationVersion": "2.8.0",
    "platform": "Linux",
    "source": "GUI"
  }
}
```

### Entry Update Transaction

```json
{
  "id": "550e8400-e29b-41d4-a716-446655440001",
  "type": "UpdateEntry",
  "timestamp": "2024-01-15T11:00:00Z",
  "description": "Update password for GitHub Account",
  "data": {
    "entryId": "entry-uuid",
    "changes": {
      "attributes": {
        "Password": {
          "value": "newstrongerpassword456",
          "protected": true
        }
      },
      "timeInfo": {
        "lastModificationTime": "2024-01-15T11:00:00Z"
      }
    },
    "previousValues": {
      "attributes": {
        "Password": {
          "value": "[PROTECTED]",
          "protected": true
        }
      },
      "timeInfo": {
        "lastModificationTime": "2024-01-15T10:30:00Z"
      }
    }
  }
}
```

### Bulk Operation Transaction

```json
{
  "id": "550e8400-e29b-41d4-a716-446655440002",
  "type": "BulkOperation",
  "timestamp": "2024-01-15T12:00:00Z",
  "description": "Import entries from CSV file",
  "data": {
    "operations": [
      {
        "type": "CreateGroup",
        "data": {
          "parentId": "root-group-uuid",
          "group": {
            "uuid": "imported-group-uuid",
            "name": "Imported Accounts",
            "iconNumber": 48
          }
        }
      },
      {
        "type": "CreateEntry",
        "data": {
          "groupId": "imported-group-uuid",
          "entry": {
            "title": "Example Service",
            "attributes": {
              "UserName": {"value": "user1", "protected": false},
              "Password": {"value": "pass1", "protected": true}
            }
          }
        }
      }
    ],
    "rollbackOnFailure": true,
    "continueOnError": false
  }
}
```

## Integration Examples

### GUI Integration Example

```cpp
// In a GUI command class
void EntryEditCommand::execute() {
    auto transaction = Transaction::updateEntryAttribute(
        m_entryId, 
        "Password", 
        m_newPassword, 
        true  // protected
    );
    transaction.setMetadata("source", "EntryEditDialog");
    transaction.setMetadata("field", "password");
    
    auto result = m_undoRedoManager->executeTransaction(transaction);
    if (!result.success) {
        // Handle error
        QMessageBox::warning(m_parent, tr("Error"), result.error);
        return;
    }
    
    // Update UI to reflect changes
    emit entryModified(m_entryId);
}
```

### CLI Integration Example

```cpp
// In CLI command processor
bool CliEdit::execute(const QStringList& arguments) {
    // Parse command line arguments
    QString entryPath = arguments.value(0);
    QString field = arguments.value(1);
    QString value = arguments.value(2);
    
    // Find entry by path
    auto entry = m_database->rootGroup()->findEntryByPath(entryPath);
    if (!entry) {
        m_errorStream << "Entry not found: " << entryPath << Qt::endl;
        return false;
    }
    
    // Create transaction
    auto transaction = Transaction::updateEntryAttribute(
        entry->uuid(), field, value, isProtectedField(field)
    );
    transaction.setMetadata("source", "CLI");
    transaction.setMetadata("command", "edit");
    
    // Apply transaction
    auto result = m_transactionProcessor->apply(transaction);
    if (!result.success) {
        m_errorStream << "Failed to update entry: " << result.error << Qt::endl;
        return false;
    }
    
    m_outputStream << "Entry updated successfully" << Qt::endl;
    return true;
}
```

### Plugin API Example

```cpp
// Plugin interface for external automation
class TransactionAPI : public QObject {
    Q_OBJECT
    
public slots:
    // High-level operations
    QString createEntry(const QString& groupPath, const QJsonObject& entryData);
    bool updateEntry(const QString& entryPath, const QJsonObject& changes);
    bool deleteEntry(const QString& entryPath);
    
    // Batch operations
    QString executeBatch(const QJsonArray& transactions);
    
    // Undo/Redo
    bool undo();
    bool redo();
    bool canUndo() const;
    bool canRedo() const;
    
    // Transaction history
    QJsonArray getTransactionHistory(int limit = 50) const;
    
signals:
    void transactionExecuted(const QString& transactionId, bool success);
    void databaseChanged();
};
```

## Performance Considerations

### Memory Optimization

```cpp
// Copy-on-write implementation for efficient immutability
template<typename T>
class CopyOnWrite {
private:
    mutable QSharedPointer<T> m_data;
    
public:
    CopyOnWrite(const T& data) : m_data(QSharedPointer<T>::create(data)) {}
    CopyOnWrite(const CopyOnWrite& other) : m_data(other.m_data) {}
    
    const T& read() const { return *m_data; }
    
    T& write() {
        if (m_data.useCount() > 1) {
            m_data = QSharedPointer<T>::create(*m_data);
        }
        return *m_data;
    }
};

// Usage in immutable classes
class ImmutableEntry {
private:
    CopyOnWrite<EntryData> m_data;
    CopyOnWrite<EntryAttributes> m_attributes;
    
public:
    ImmutableEntry withTitle(const QString& title) const {
        ImmutableEntry copy(*this);
        copy.m_data.write().title = title;
        return copy;
    }
};
```

### Transaction Compression

```cpp
// Compress transaction history for long-term storage
class TransactionCompressor {
public:
    static QByteArray compress(const QList<Transaction>& transactions) {
        QJsonArray array;
        for (const auto& tx : transactions) {
            array.append(tx.toJson());
        }
        
        QJsonDocument doc(array);
        return qCompress(doc.toJson(QJsonDocument::Compact));
    }
    
    static QList<Transaction> decompress(const QByteArray& data) {
        QByteArray uncompressed = qUncompress(data);
        QJsonDocument doc = QJsonDocument::fromJson(uncompressed);
        
        QList<Transaction> transactions;
        for (const auto& value : doc.array()) {
            transactions.append(Transaction::fromJson(value.toObject()));
        }
        return transactions;
    }
};
```

This technical specification provides the detailed implementation guidance needed to move forward with the transactional edits architecture in KeePassXC.