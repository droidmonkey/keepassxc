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

#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>

#include "ImmutableEntry.h"
#include "Transaction.h"
#include "TransactionManager.h"

/**
 * Example demonstrating the transactional architecture for KeePassXC
 * This shows how the new immutable data structures and transaction system work
 */

void demonstrateImmutableEntry()
{
    qDebug() << "=== Demonstrating Immutable Entry ===";

    // Create a new entry
    ImmutableEntry entry;
    qDebug() << "Created entry with UUID:" << entry.uuid().toString();

    // Configure the entry using fluent interface
    ImmutableEntry githubEntry = entry.withTitle("GitHub Account")
                                     .withUsername("john.doe@example.com")
                                     .withPassword("SecretPassword123")
                                     .withUrl("https://github.com")
                                     .withAttribute("Notes", "Work account", false)
                                     .withAttribute("2FA", "TOTP", false);

    qDebug() << "Configured entry:";
    qDebug() << "  Title:" << githubEntry.title();
    qDebug() << "  Username:" << githubEntry.username();
    qDebug() << "  URL:" << githubEntry.url();
    qDebug() << "  Notes:" << githubEntry.attribute("Notes");
    qDebug() << "  2FA:" << githubEntry.attribute("2FA");

    // Original entry remains unchanged
    qDebug() << "Original entry title (should be empty):" << entry.title();

    // Demonstrate copy-on-write
    ImmutableEntry copy = githubEntry;
    ImmutableEntry modified = copy.withPassword("NewPassword456");

    qDebug() << "Original password:" << githubEntry.password();
    qDebug() << "Modified password:" << modified.password();
    qDebug() << "Copy password:" << copy.password();

    // All three should have the same UUID
    qDebug() << "All UUIDs match:" << (entry.uuid() == githubEntry.uuid() && githubEntry.uuid() == modified.uuid());
}

void demonstrateTransactions()
{
    qDebug() << "\n=== Demonstrating Transactions ===";

    // Create a transaction to update an entry
    Transaction transaction(TransactionType::UpdateEntry, "Update GitHub password");

    QUuid entryId = QUuid::createUuid();
    transaction.setEntryTarget(entryId);
    transaction.setAttributeChange("Password", "NewSecretPassword789", true);
    transaction.setPropertyChange("modified", QDateTime::currentDateTimeUtc());

    qDebug() << "Created transaction:";
    qDebug() << "  ID:" << transaction.id().toString();
    qDebug() << "  Type:" << static_cast<int>(transaction.type());
    qDebug() << "  Description:" << transaction.description();
    qDebug() << "  Target Entry:" << transaction.target("entryId").toString();
    qDebug() << "  Valid:" << transaction.isValid();

    // Serialize to JSON
    QJsonObject json = transaction.toJson();
    QJsonDocument doc(json);
    qDebug() << "JSON representation:";
    qDebug() << doc.toJson(QJsonDocument::Indented);

    // Deserialize from JSON
    Transaction deserialized(json);
    qDebug() << "Deserialized transaction valid:" << deserialized.isValid();
    qDebug() << "Round-trip successful:" << (transaction.id() == deserialized.id());
}

void demonstrateTransactionManager()
{
    qDebug() << "\n=== Demonstrating Transaction Manager ===";

    // Create a mock database (normally this would be a real Database instance)
    Database* database = nullptr; // In real implementation, this would be valid

    TransactionManager manager(database);

    // Create some sample transactions
    Transaction createTransaction(TransactionType::CreateEntry, "Create new entry");
    createTransaction.setEntryTarget(QUuid::createUuid());
    createTransaction.setAttributeChange("Title", "New Entry", false);
    createTransaction.setAttributeChange("Password", "InitialPassword", true);

    Transaction updateTransaction(TransactionType::UpdateEntry, "Update password");
    updateTransaction.setEntryTarget(QUuid::createUuid());
    updateTransaction.setAttributeChange("Password", "UpdatedPassword", true);

    qDebug() << "Initial state:";
    qDebug() << "  Can undo:" << manager.canUndo();
    qDebug() << "  Can redo:" << manager.canRedo();
    qDebug() << "  History size:" << manager.historySize();

    // Note: In a real implementation, these would actually modify the database
    // For this demonstration, we're just showing the transaction structure

    qDebug() << "\nTransactions created successfully";
    qDebug() << "In a full implementation, these would be executed and recorded";
}

void demonstrateUsagePatterns()
{
    qDebug() << "\n=== Demonstrating Usage Patterns ===";

    // Pattern 1: Simple attribute update
    qDebug() << "Pattern 1: Simple attribute update";
    ImmutableEntry entry;
    ImmutableEntry updated = entry.withTitle("My Account").withPassword("MyPassword123");
    qDebug() << "  Updated entry created with title:" << updated.title();

    // Pattern 2: Chained modifications
    qDebug() << "Pattern 2: Chained modifications";
    ImmutableEntry complex = entry.withTitle("Complex Entry")
                                 .withUsername("user@example.com")
                                 .withUrl("https://example.com")
                                 .withAttribute("Category", "Work", false)
                                 .withAttribute("Notes", "Important account", false);

    qDebug() << "  Complex entry has" << complex.attributes().size() << "custom attributes";

    // Pattern 3: Conditional modifications
    qDebug() << "Pattern 3: Conditional modifications";
    ImmutableEntry conditional = entry.withTitle("Conditional Entry");
    if (conditional.title().contains("Conditional")) {
        conditional = conditional.withAttribute("Type", "Conditional", false);
    }
    qDebug() << "  Conditional entry type:" << conditional.attribute("Type");

    // Pattern 4: Bulk operations preparation
    qDebug() << "Pattern 4: Bulk operations (transaction list)";
    QVector<Transaction> bulkTransactions;

    for (int i = 0; i < 3; ++i) {
        Transaction tx(TransactionType::CreateEntry, QString("Create entry %1").arg(i + 1));
        tx.setEntryTarget(QUuid::createUuid());
        tx.setAttributeChange("Title", QString("Entry %1").arg(i + 1), false);
        tx.setAttributeChange("Password", QString("Password%1").arg(i + 1), true);
        bulkTransactions.append(tx);
    }

    qDebug() << "  Created" << bulkTransactions.size() << "bulk transactions";
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "KeePassXC Transactional Architecture Demo";
    qDebug() << "=========================================";

    demonstrateImmutableEntry();
    demonstrateTransactions();
    demonstrateTransactionManager();
    demonstrateUsagePatterns();

    qDebug() << "\nDemo completed successfully!";
    qDebug() << "This demonstrates the core concepts of the transactional architecture.";
    qDebug() << "In a full implementation, these would integrate with the actual Database class.";

    return 0;
}