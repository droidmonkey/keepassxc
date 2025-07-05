# Proof of Concept: Transactional Entry Operations

## Overview

This document demonstrates a proof-of-concept implementation of transactional operations for KeePassXC entries. The implementation shows how to retrofit transactional behavior onto the existing Entry class while maintaining backward compatibility.

## Proof of Concept Implementation

### Transaction-Aware Entry Wrapper

```cpp
// src/core/transactions/TransactionalEntry.h
#ifndef KEEPASSXC_TRANSACTIONALENTRY_H
#define KEEPASSXC_TRANSACTIONALENTRY_H

#include "core/Entry.h"
#include "Transaction.h"
#include <QSharedPointer>
#include <QJsonObject>

namespace Transactions {

/**
 * Wrapper around Entry that records all modifications as transactions
 * This demonstrates how to add transactional behavior to existing classes
 */
class TransactionalEntry : public QObject {
    Q_OBJECT

public:
    explicit TransactionalEntry(QSharedPointer<Entry> entry, QObject* parent = nullptr);
    
    // Read-only access to underlying entry
    QSharedPointer<const Entry> entry() const { return m_entry; }
    
    // Transactional modification methods
    Transaction setTitle(const QString& title, const QString& description = QString());
    Transaction setUsername(const QString& username, const QString& description = QString());
    Transaction setPassword(const QString& password, const QString& description = QString());
    Transaction setUrl(const QString& url, const QString& description = QString());
    Transaction setNotes(const QString& notes, const QString& description = QString());
    Transaction setAttribute(const QString& key, const QString& value, bool isProtected = false, const QString& description = QString());
    
    // Transactional operations
    Transaction addToGroup(const QUuid& groupId, const QString& description = QString());
    Transaction removeFromGroup(const QString& description = QString());
    Transaction addTag(const QString& tag, const QString& description = QString());
    Transaction removeTag(const QString& tag, const QString& description = QString());
    
    // Bulk operations
    Transaction updateMultipleAttributes(const QMap<QString, QString>& attributes, const QString& description = QString());
    
    // Transaction application
    bool applyTransaction(const Transaction& transaction);
    QJsonObject getCurrentState() const;
    
signals:
    void transactionCreated(const Transaction& transaction);
    void transactionApplied(const Transaction& transaction, bool success);

private:
    QSharedPointer<Entry> m_entry;
    
    // Helper methods
    QJsonObject createAttributeChange(const QString& key, const QString& value, bool isProtected = false) const;
    QJsonObject createBasicFieldChange(const QString& field, const QString& value) const;
    Transaction createTransaction(TransactionType type, const QJsonObject& data, const QString& description) const;
    
    // Transaction validators
    bool validateAttributeChange(const QJsonObject& data) const;
    bool validateBasicFieldChange(const QJsonObject& data) const;
};

} // namespace Transactions

#endif // KEEPASSXC_TRANSACTIONALENTRY_H
```

### Implementation Example

```cpp
// src/core/transactions/TransactionalEntry.cpp
#include "TransactionalEntry.h"
#include "core/Clock.h"
#include <QJsonDocument>

namespace Transactions {

TransactionalEntry::TransactionalEntry(QSharedPointer<Entry> entry, QObject* parent)
    : QObject(parent), m_entry(entry)
{
    Q_ASSERT(entry);
}

Transaction TransactionalEntry::setTitle(const QString& title, const QString& description)
{
    QJsonObject data = createBasicFieldChange("title", title);
    QString desc = description.isEmpty() ? 
        tr("Change entry title to '%1'").arg(title) : description;
    
    auto transaction = createTransaction(TransactionType::UpdateEntry, data, desc);
    emit transactionCreated(transaction);
    return transaction;
}

Transaction TransactionalEntry::setPassword(const QString& password, const QString& description)
{
    QJsonObject data = createAttributeChange("Password", password, true);
    QString desc = description.isEmpty() ? 
        tr("Update entry password") : description;
    
    auto transaction = createTransaction(TransactionType::UpdateEntry, data, desc);
    emit transactionCreated(transaction);
    return transaction;
}

Transaction TransactionalEntry::setAttribute(const QString& key, const QString& value, 
                                           bool isProtected, const QString& description)
{
    QJsonObject data = createAttributeChange(key, value, isProtected);
    QString desc = description.isEmpty() ? 
        tr("Update entry attribute '%1'").arg(key) : description;
    
    auto transaction = createTransaction(TransactionType::UpdateEntry, data, desc);
    emit transactionCreated(transaction);
    return transaction;
}

Transaction TransactionalEntry::updateMultipleAttributes(const QMap<QString, QString>& attributes, 
                                                       const QString& description)
{
    QJsonObject data;
    data["entryId"] = m_entry->uuid().toString();
    
    QJsonObject changes;
    QJsonObject attributeChanges;
    
    for (auto it = attributes.begin(); it != attributes.end(); ++it) {
        QJsonObject attrData;
        attrData["value"] = it.value();
        attrData["protected"] = (it.key() == "Password"); // Basic heuristic
        attributeChanges[it.key()] = attrData;
    }
    
    changes["attributes"] = attributeChanges;
    changes["timeInfo"] = QJsonObject{{"lastModificationTime", Clock::currentDateTimeUtc().toString(Qt::ISODate)}};
    data["changes"] = changes;
    
    QString desc = description.isEmpty() ? 
        tr("Update %1 entry attributes").arg(attributes.size()) : description;
    
    auto transaction = createTransaction(TransactionType::UpdateEntry, data, desc);
    emit transactionCreated(transaction);
    return transaction;
}

bool TransactionalEntry::applyTransaction(const Transaction& transaction)
{
    if (transaction.type() != TransactionType::UpdateEntry) {
        return false;
    }
    
    QJsonObject data = transaction.data();
    if (!data.contains("entryId") || data["entryId"].toString() != m_entry->uuid().toString()) {
        return false;
    }
    
    QJsonObject changes = data["changes"].toObject();
    bool success = true;
    
    // Apply basic field changes
    if (changes.contains("title")) {
        m_entry->setTitle(changes["title"].toString());
    }
    if (changes.contains("notes")) {
        m_entry->setNotes(changes["notes"].toString());
    }
    if (changes.contains("url")) {
        m_entry->setUrl(changes["url"].toString());
    }
    
    // Apply attribute changes
    if (changes.contains("attributes")) {
        QJsonObject attributes = changes["attributes"].toObject();
        for (auto it = attributes.begin(); it != attributes.end(); ++it) {
            QJsonObject attrData = it.value().toObject();
            m_entry->attributes()->set(it.key(), attrData["value"].toString(), attrData["protected"].toBool());
        }
    }
    
    // Apply time info changes
    if (changes.contains("timeInfo")) {
        QJsonObject timeInfo = changes["timeInfo"].toObject();
        if (timeInfo.contains("lastModificationTime")) {
            // Time info is updated automatically by the Entry class
        }
    }
    
    emit transactionApplied(transaction, success);
    return success;
}

QJsonObject TransactionalEntry::getCurrentState() const
{
    QJsonObject state;
    state["uuid"] = m_entry->uuid().toString();
    state["title"] = m_entry->title();
    state["notes"] = m_entry->notes();
    state["url"] = m_entry->url();
    
    // Attributes
    QJsonObject attributes;
    auto entryAttributes = m_entry->attributes();
    for (const QString& key : entryAttributes->keys()) {
        QJsonObject attrData;
        attrData["value"] = entryAttributes->value(key);
        attrData["protected"] = entryAttributes->isProtected(key);
        attributes[key] = attrData;
    }
    state["attributes"] = attributes;
    
    // Time info
    QJsonObject timeInfo;
    timeInfo["creationTime"] = m_entry->timeInfo().creationTime().toString(Qt::ISODate);
    timeInfo["lastModificationTime"] = m_entry->timeInfo().lastModificationTime().toString(Qt::ISODate);
    timeInfo["lastAccessTime"] = m_entry->timeInfo().lastAccessTime().toString(Qt::ISODate);
    state["timeInfo"] = timeInfo;
    
    return state;
}

// Private helper methods
QJsonObject TransactionalEntry::createAttributeChange(const QString& key, const QString& value, bool isProtected) const
{
    QJsonObject data;
    data["entryId"] = m_entry->uuid().toString();
    
    QJsonObject changes;
    QJsonObject attributes;
    QJsonObject attrData;
    attrData["value"] = value;
    attrData["protected"] = isProtected;
    attributes[key] = attrData;
    changes["attributes"] = attributes;
    changes["timeInfo"] = QJsonObject{{"lastModificationTime", Clock::currentDateTimeUtc().toString(Qt::ISODate)}};
    
    data["changes"] = changes;
    return data;
}

QJsonObject TransactionalEntry::createBasicFieldChange(const QString& field, const QString& value) const
{
    QJsonObject data;
    data["entryId"] = m_entry->uuid().toString();
    
    QJsonObject changes;
    changes[field] = value;
    changes["timeInfo"] = QJsonObject{{"lastModificationTime", Clock::currentDateTimeUtc().toString(Qt::ISODate)}};
    
    data["changes"] = changes;
    return data;
}

Transaction TransactionalEntry::createTransaction(TransactionType type, const QJsonObject& data, const QString& description) const
{
    Transaction transaction(type, data, description);
    transaction.setMetadata("entryId", m_entry->uuid().toString());
    transaction.setMetadata("entryTitle", m_entry->title());
    return transaction;
}

} // namespace Transactions
```

### Simple Transaction Manager

```cpp
// src/core/transactions/SimpleTransactionManager.h
#ifndef KEEPASSXC_SIMPLETRANSACTIONMANAGER_H
#define KEEPASSXC_SIMPLETRANSACTIONMANAGER_H

#include "Transaction.h"
#include "TransactionalEntry.h"
#include <QObject>
#include <QList>
#include <QJsonObject>

namespace Transactions {

/**
 * Simple transaction manager that demonstrates basic undo/redo functionality
 * This is a proof-of-concept implementation
 */
class SimpleTransactionManager : public QObject {
    Q_OBJECT

public:
    explicit SimpleTransactionManager(QObject* parent = nullptr);
    
    // Transaction execution
    bool executeTransaction(const Transaction& transaction);
    bool executeBatch(const QList<Transaction>& transactions);
    
    // Undo/Redo
    bool canUndo() const { return !m_undoStack.isEmpty(); }
    bool canRedo() const { return !m_redoStack.isEmpty(); }
    bool undo();
    bool redo();
    
    // History
    QList<Transaction> undoHistory() const { return m_undoStack; }
    QList<Transaction> redoHistory() const { return m_redoStack; }
    void clearHistory();
    
    // Entry management
    void registerEntry(QSharedPointer<TransactionalEntry> entry);
    void unregisterEntry(const QUuid& entryId);
    
signals:
    void transactionExecuted(const Transaction& transaction, bool success);
    void undoPerformed(const Transaction& transaction);
    void redoPerformed(const Transaction& transaction);
    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);

private:
    QList<Transaction> m_undoStack;
    QList<Transaction> m_redoStack;
    QHash<QUuid, QSharedPointer<TransactionalEntry>> m_entries;
    
    // Helper methods
    Transaction createInverseTransaction(const Transaction& original);
    QSharedPointer<TransactionalEntry> findEntry(const QUuid& entryId);
    void updateUndoRedoState();
};

} // namespace Transactions

#endif // KEEPASSXC_SIMPLETRANSACTIONMANAGER_H
```

### Usage Example

```cpp
// Example usage in GUI code
void EntryEditDialog::saveChanges()
{
    if (!m_transactionalEntry) return;
    
    // Create transactions for each changed field
    QList<Transaction> transactions;
    
    if (m_titleChanged) {
        transactions.append(m_transactionalEntry->setTitle(m_titleEdit->text()));
    }
    
    if (m_passwordChanged) {
        transactions.append(m_transactionalEntry->setPassword(m_passwordEdit->text()));
    }
    
    if (m_notesChanged) {
        transactions.append(m_transactionalEntry->setNotes(m_notesEdit->toPlainText()));
    }
    
    // Execute as a batch operation
    if (!transactions.isEmpty()) {
        bool success = m_transactionManager->executeBatch(transactions);
        if (success) {
            accept(); // Close dialog
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to save changes"));
        }
    }
}

// Undo/Redo menu actions
void MainWindow::onUndo()
{
    if (m_transactionManager->canUndo()) {
        m_transactionManager->undo();
        updateUndoRedoActions();
    }
}

void MainWindow::onRedo()
{
    if (m_transactionManager->canRedo()) {
        m_transactionManager->redo();
        updateUndoRedoActions();
    }
}

void MainWindow::updateUndoRedoActions()
{
    m_undoAction->setEnabled(m_transactionManager->canUndo());
    m_redoAction->setEnabled(m_transactionManager->canRedo());
    
    if (m_transactionManager->canUndo()) {
        auto history = m_transactionManager->undoHistory();
        m_undoAction->setText(tr("Undo: %1").arg(history.last().description()));
    } else {
        m_undoAction->setText(tr("Undo"));
    }
    
    if (m_transactionManager->canRedo()) {
        auto history = m_transactionManager->redoHistory();
        m_redoAction->setText(tr("Redo: %1").arg(history.last().description()));
    } else {
        m_redoAction->setText(tr("Redo"));
    }
}
```

## Testing the Proof of Concept

### Unit Test Example

```cpp
// tests/TestTransactionalEntry.cpp
#include <QtTest>
#include "core/transactions/TransactionalEntry.h"
#include "core/Entry.h"
#include "core/Group.h"

class TestTransactionalEntry : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testSetTitle();
    void testSetPassword();
    void testMultipleAttributes();
    void testTransactionSerialization();
    void testUndoRedo();
    void cleanupTestCase();

private:
    QSharedPointer<Entry> m_entry;
    QSharedPointer<Transactions::TransactionalEntry> m_transactionalEntry;
    QSharedPointer<Transactions::SimpleTransactionManager> m_transactionManager;
};

void TestTransactionalEntry::initTestCase()
{
    m_entry = QSharedPointer<Entry>::create();
    m_entry->setTitle("Test Entry");
    m_entry->setUsername("testuser");
    m_entry->setPassword("testpass");
    
    m_transactionalEntry = QSharedPointer<Transactions::TransactionalEntry>::create(m_entry);
    m_transactionManager = QSharedPointer<Transactions::SimpleTransactionManager>::create();
    m_transactionManager->registerEntry(m_transactionalEntry);
}

void TestTransactionalEntry::testSetTitle()
{
    QString originalTitle = m_entry->title();
    QString newTitle = "New Test Title";
    
    // Create transaction
    auto transaction = m_transactionalEntry->setTitle(newTitle);
    
    QCOMPARE(transaction.type(), Transactions::TransactionType::UpdateEntry);
    QVERIFY(!transaction.description().isEmpty());
    
    // Execute transaction
    bool success = m_transactionManager->executeTransaction(transaction);
    QVERIFY(success);
    QCOMPARE(m_entry->title(), newTitle);
    
    // Test undo
    QVERIFY(m_transactionManager->canUndo());
    m_transactionManager->undo();
    QCOMPARE(m_entry->title(), originalTitle);
    
    // Test redo
    QVERIFY(m_transactionManager->canRedo());
    m_transactionManager->redo();
    QCOMPARE(m_entry->title(), newTitle);
}

void TestTransactionalEntry::testSetPassword()
{
    QString originalPassword = m_entry->password();
    QString newPassword = "newpassword123";
    
    // Create transaction
    auto transaction = m_transactionalEntry->setPassword(newPassword);
    
    // Execute transaction
    bool success = m_transactionManager->executeTransaction(transaction);
    QVERIFY(success);
    QCOMPARE(m_entry->password(), newPassword);
    
    // Verify password is marked as protected
    QVERIFY(m_entry->attributes()->isProtected("Password"));
}

void TestTransactionalEntry::testMultipleAttributes()
{
    QMap<QString, QString> attributes;
    attributes["Email"] = "test@example.com";
    attributes["Phone"] = "123-456-7890";
    attributes["Notes"] = "Updated notes";
    
    // Create bulk transaction
    auto transaction = m_transactionalEntry->updateMultipleAttributes(attributes);
    
    // Execute transaction
    bool success = m_transactionManager->executeTransaction(transaction);
    QVERIFY(success);
    
    // Verify all attributes were updated
    QCOMPARE(m_entry->attributes()->value("Email"), QString("test@example.com"));
    QCOMPARE(m_entry->attributes()->value("Phone"), QString("123-456-7890"));
    QCOMPARE(m_entry->notes(), QString("Updated notes"));
}

void TestTransactionalEntry::testTransactionSerialization()
{
    auto transaction = m_transactionalEntry->setTitle("Serialization Test");
    
    // Test JSON serialization
    QJsonObject json = transaction.toJson();
    QVERIFY(json.contains("id"));
    QVERIFY(json.contains("type"));
    QVERIFY(json.contains("timestamp"));
    QVERIFY(json.contains("data"));
    
    // Test deserialization
    auto deserializedTransaction = Transactions::Transaction::fromJson(json);
    QCOMPARE(deserializedTransaction.id(), transaction.id());
    QCOMPARE(deserializedTransaction.type(), transaction.type());
    QCOMPARE(deserializedTransaction.description(), transaction.description());
}

QTEST_MAIN(TestTransactionalEntry)
#include "TestTransactionalEntry.moc"
```

## Benefits Demonstrated

This proof of concept demonstrates several key benefits of the transactional approach:

1. **Automatic Transaction Generation**: All modifications create transaction objects automatically
2. **Undo/Redo Support**: Complete operations can be reversed precisely
3. **Batch Operations**: Multiple changes can be applied atomically
4. **Serialization**: Transactions can be saved and replayed
5. **Backward Compatibility**: Existing Entry objects continue to work
6. **Testability**: Transaction logic can be unit tested in isolation

## Next Steps

1. **Extend to Groups**: Implement TransactionalGroup wrapper
2. **Database Integration**: Add transaction support to Database class
3. **GUI Integration**: Update edit dialogs to use transactional operations
4. **Performance Testing**: Measure overhead of transaction creation
5. **Full Implementation**: Replace proof of concept with production code

This proof of concept provides a solid foundation for implementing the full transactional architecture in KeePassXC.