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
#include <QUuid>

#include "DirectTransaction.h"
#include "DirectTransactionManager.h"

// Mock Database class for demonstration
class Database : public QObject
{
    Q_OBJECT
public:
    Database(QObject* parent = nullptr)
        : QObject(parent)
    {
    }
};

#include "DirectTransactionDemo.moc"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== Direct Transaction Manager Demo ===\n";

    // Create a mock database for demonstration
    Database* database = new Database();
    DirectTransactionManager* manager = new DirectTransactionManager(database);

    // Demo 1: Basic entry operations using convenience methods
    qDebug() << "Demo 1: Basic Entry Operations";
    qDebug() << "------------------------------";

    QUuid groupId = QUuid::createUuid();
    QUuid entryId = QUuid::createUuid();

    // Create an entry using convenience method
    qDebug() << "Creating entry with convenience method...";
    bool success = manager->createEntry(groupId, "GitHub Account", "user@example.com", "password123");
    qDebug() << "Result:" << (success ? "SUCCESS" : "FAILED");

    // Update entry using convenience method
    qDebug() << "Updating entry title...";
    success = manager->updateEntryTitle(entryId, "GitHub Enterprise Account");
    qDebug() << "Result:" << (success ? "SUCCESS" : "FAILED");

    qDebug() << "";

    // Demo 2: Manual transaction creation
    qDebug() << "Demo 2: Manual Transaction Creation";
    qDebug() << "-----------------------------------";

    // Create a transaction manually for more control
    DirectTransaction updateTransaction(DirectTransactionType::UpdateEntry, "Update GitHub password");
    updateTransaction.setEntryTarget(entryId);
    updateTransaction.setEntryAttribute("Password", "newSecurePassword456", true);
    updateTransaction.setEntryAttribute("URL", "https://github.com/company", false);

    qDebug() << "Executing manual transaction...";
    success = manager->executeTransaction(updateTransaction);
    qDebug() << "Result:" << (success ? "SUCCESS" : "FAILED");

    qDebug() << "";

    // Demo 3: Undo/Redo operations
    qDebug() << "Demo 3: Undo/Redo Operations";
    qDebug() << "-----------------------------";

    qDebug() << "Can undo:" << manager->canUndo();
    qDebug() << "Can redo:" << manager->canRedo();
    qDebug() << "History size:" << manager->historySize();

    if (manager->canUndo()) {
        qDebug() << "Performing undo...";
        success = manager->undo();
        qDebug() << "Undo result:" << (success ? "SUCCESS" : "FAILED");
    }

    if (manager->canRedo()) {
        qDebug() << "Performing redo...";
        success = manager->redo();
        qDebug() << "Redo result:" << (success ? "SUCCESS" : "FAILED");
    }

    qDebug() << "";

    // Demo 4: Batch operations
    qDebug() << "Demo 4: Batch Operations";
    qDebug() << "------------------------";

    manager->beginBatch("Update multiple entries");

    // Batch multiple operations
    manager->updateEntryAttribute(entryId, "Notes", "Updated in batch operation");
    manager->updateEntryAttribute(entryId, "Email", "newemail@company.com");
    manager->updateEntryTitle(entryId, "Updated GitHub Enterprise");

    qDebug() << "Batch operations queued, ending batch...";
    manager->endBatch();

    qDebug() << "";

    // Demo 5: Transaction validation
    qDebug() << "Demo 5: Transaction Validation";
    qDebug() << "------------------------------";

    // Create an invalid transaction
    DirectTransaction invalidTransaction(DirectTransactionType::UpdateEntry, "Invalid transaction");
    // Don't set any target or changes

    qDebug() << "Invalid transaction valid:" << invalidTransaction.isValid();
    qDebug() << "Validation error:" << invalidTransaction.validationError();

    // Create a valid transaction
    DirectTransaction validTransaction(DirectTransactionType::UpdateEntry, "Valid transaction");
    validTransaction.setEntryTarget(entryId);
    validTransaction.setEntryAttribute("LastModified", QDateTime::currentDateTime().toString());

    qDebug() << "Valid transaction valid:" << validTransaction.isValid();
    qDebug() << "Validation error:" << validTransaction.validationError();

    qDebug() << "";

    // Demo 6: Comparing approaches
    qDebug() << "Demo 6: Comparing Direct vs JSON Approaches";
    qDebug() << "--------------------------------------------";

    qDebug() << "Direct approach:";
    qDebug() << "  - Type-safe structs for targets and changes";
    qDebug() << "  - No JSON serialization overhead";
    qDebug() << "  - Compile-time validation";
    qDebug() << "  - Direct member access";

    qDebug() << "\nJSON approach:";
    qDebug() << "  - Flexible key-value storage";
    qDebug() << "  - Runtime validation";
    qDebug() << "  - Serialization for persistence/networking";
    qDebug() << "  - Dynamic property access";

    qDebug() << "";

    // Demo 7: Performance comparison setup
    qDebug() << "Demo 7: Performance Characteristics";
    qDebug() << "-----------------------------------";

    qDebug() << "Direct transactions:";
    qDebug() << "  + Faster execution (no JSON parsing)";
    qDebug() << "  + Lower memory overhead";
    qDebug() << "  + Type safety at compile time";
    qDebug() << "  - Less flexible for dynamic use cases";

    qDebug() << "\nJSON transactions:";
    qDebug() << "  + More flexible for scripting/external tools";
    qDebug() << "  + Easier to serialize for storage";
    qDebug() << "  + Dynamic property handling";
    qDebug() << "  - Runtime overhead from JSON processing";

    qDebug() << "";

    // Demo 8: History and audit trail
    qDebug() << "Demo 8: History and Audit Trail";
    qDebug() << "-------------------------------";

    auto history = manager->getHistory(5);
    qDebug() << "Transaction history (last 5):";
    for (int i = 0; i < history.size(); ++i) {
        const auto& transaction = history[i];
        qDebug() << QString("  %1. %2 (%3)")
                        .arg(i + 1)
                        .arg(transaction.description())
                        .arg(transaction.timestamp().toString());
    }

    auto auditTrail = manager->getAuditTrail();
    qDebug() << "\nAudit trail contains" << auditTrail.size() << "transactions";

    qDebug() << "\n=== Demo Complete ===";

    // Cleanup
    delete manager;
    delete database;

    return 0;
}