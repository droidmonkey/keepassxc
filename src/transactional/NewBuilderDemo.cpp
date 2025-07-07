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

#include "Builder.h"
#include <QCoreApplication>
#include <QDebug>
#include <QMap>
#include <QUuid>

void demonstrateEntryTransactions()
{
    qDebug() << "\n=== Entry Transaction Builder Demo ===";

    QUuid rootGroupId = QUuid::createUuid();
    QUuid existingEntryId = QUuid::createUuid();

    // Example 1: Create a new entry with auto-generated UUID
    qDebug() << "\n1. Creating new entry with basic attributes:";
    try {
        auto transaction = Builder::createEntry(rootGroupId)
                               .withDescription("Create new login entry")
                               .withTitle("GitHub Account")
                               .withUsername("johndoe@example.com")
                               .withPassword("SecurePassword123!")
                               .withUrl("https://github.com")
                               .withNotes("Work GitHub account")
                               .withIcon(1)
                               .build();

        qDebug() << "   Transaction ID:" << transaction.id().toString();
        qDebug() << "   Description:" << transaction.description();
        qDebug() << "   Entry ID:" << transaction.entryTarget().entryId.toString();
        qDebug() << "   Parent Group:" << transaction.entryTarget().groupId.toString();

        auto changes = transaction.entryChanges();
        qDebug() << "   Attributes:" << changes.attributes;
        qDebug() << "   Protected:" << changes.protectedAttributes;

    } catch (const std::exception& e) {
        qDebug() << "   Error:" << e.what();
    }

    // Example 2: Update entry with bulk attributes
    qDebug() << "\n2. Updating entry with bulk attribute operations:";
    try {
        QMap<QString, QString> customAttribs;
        customAttribs["Department"] = "Engineering";
        customAttribs["Employee ID"] = "ENG-001";

        QMap<QString, QString> protectedAttribs;
        protectedAttribs["API Key"] = "secret-key-12345";
        protectedAttribs["Private Token"] = "private-token-67890";

        auto transaction = Builder::updateEntry(existingEntryId)
                               .withDescription("Update with bulk attributes")
                               .withAttributes(customAttribs)
                               .withProtectedAttributes(protectedAttribs)
                               .withTags(QStringList{"work", "api", "important"})
                               .withForegroundColor("#FF0000")
                               .withAutoTypeEnabled(true)
                               .build();

        qDebug() << "   Transaction ID:" << transaction.id().toString();

        auto changes = transaction.entryChanges();
        qDebug() << "   All attributes:" << changes.attributes;
        qDebug() << "   Protected status:" << changes.protectedAttributes;
        qDebug() << "   Properties:" << changes.properties;

    } catch (const std::exception& e) {
        qDebug() << "   Error:" << e.what();
    }

    // Example 3: Clear attributes operation
    qDebug() << "\n3. Clearing specific attributes:";
    try {
        auto transaction = Builder::updateEntry(existingEntryId)
                               .withDescription("Clear outdated attributes")
                               .clearAttributes(QStringList{"Old Field", "Deprecated"})
                               .withNotes("Updated notes after cleanup")
                               .build();

        qDebug() << "   Transaction ID:" << transaction.id().toString();
        qDebug() << "   Description:" << transaction.description();

    } catch (const std::exception& e) {
        qDebug() << "   Error:" << e.what();
    }

    // Example 4: Move entry to different group
    qDebug() << "\n4. Moving entry to different group:";
    try {
        QUuid targetGroupId = QUuid::createUuid();

        auto transaction =
            Builder::moveEntry(existingEntryId).withDescription("Move to archive group").toGroup(targetGroupId).build();

        qDebug() << "   Transaction ID:" << transaction.id().toString();
        qDebug() << "   Entry ID:" << transaction.entryTarget().entryId.toString();
        qDebug() << "   Target Group:" << transaction.entryTarget().groupId.toString();

    } catch (const std::exception& e) {
        qDebug() << "   Error:" << e.what();
    }
}

void demonstrateGroupTransactions()
{
    qDebug() << "\n=== Group Transaction Builder Demo ===";

    QUuid rootGroupId = QUuid::createUuid();
    QUuid existingGroupId = QUuid::createUuid();

    // Example 1: Create a new group with auto-generated UUID
    qDebug() << "\n1. Creating new group:";
    try {
        auto transaction = Builder::createGroup(rootGroupId)
                               .withDescription("Create project folder")
                               .withName("Web Development")
                               .withNotes("All web development related credentials")
                               .withIcon(2)
                               .withExpanded(true)
                               .withAutoTypeEnabled(GroupTransactionBuilder::TriState::Enable)
                               .withSearchingEnabled(GroupTransactionBuilder::TriState::Enable)
                               .build();

        qDebug() << "   Transaction ID:" << transaction.id().toString();
        qDebug() << "   Description:" << transaction.description();
        qDebug() << "   Group ID:" << transaction.groupTarget().groupId.toString();
        qDebug() << "   Parent ID:" << transaction.groupTarget().parentId.toString();

        auto changes = transaction.groupChanges();
        qDebug() << "   Properties:" << changes.properties;

    } catch (const std::exception& e) {
        qDebug() << "   Error:" << e.what();
    }

    // Example 2: Update group with bulk properties
    qDebug() << "\n2. Updating group with bulk properties:";
    try {
        QMap<QString, QVariant> properties;
        properties["tags"] = "project,development,web";
        properties["defaultAutoTypeSequence"] = "{USERNAME}{TAB}{PASSWORD}{ENTER}";
        properties["isExpanded"] = false;

        auto transaction = Builder::updateGroup(existingGroupId)
                               .withDescription("Update group configuration")
                               .withProperties(properties)
                               .withMergeMode(GroupTransactionBuilder::MergeMode::Synchronize)
                               .withCustomData("project_id", "WEB-2024-001")
                               .withCustomData("team", "frontend")
                               .build();

        qDebug() << "   Transaction ID:" << transaction.id().toString();

        auto changes = transaction.groupChanges();
        qDebug() << "   All properties:" << changes.properties;

    } catch (const std::exception& e) {
        qDebug() << "   Error:" << e.what();
    }

    // Example 3: Move group to different parent
    qDebug() << "\n3. Moving group to different parent:";
    try {
        QUuid newParentId = QUuid::createUuid();

        auto transaction = Builder::moveGroup(existingGroupId)
                               .withDescription("Reorganize folder structure")
                               .toParent(newParentId)
                               .build();

        qDebug() << "   Transaction ID:" << transaction.id().toString();
        qDebug() << "   Group ID:" << transaction.groupTarget().groupId.toString();
        qDebug() << "   New Parent:" << transaction.groupTarget().parentId.toString();

    } catch (const std::exception& e) {
        qDebug() << "   Error:" << e.what();
    }
}

void demonstrateValidationAndErrors()
{
    qDebug() << "\n=== Validation and Error Handling Demo ===";

    // Example 1: Try to build without required changes
    qDebug() << "\n1. Update without any changes (should fail):";
    try {
        auto builder = Builder::updateEntry(QUuid::createUuid());
        if (!builder.isValid()) {
            qDebug() << "   Validation failed:" << builder.validationError();
        }
        // This will throw because it's invalid
        auto transaction = builder.build();

    } catch (const std::exception& e) {
        qDebug() << "   Caught expected error:" << e.what();
    }

    // Example 2: Try to use after building
    qDebug() << "\n2. Reuse builder after build (should fail):";
    try {
        auto builder = Builder::updateEntry(QUuid::createUuid()).withTitle("Test");
        auto transaction = builder.build(); // First build succeeds

        // This should fail
        builder.withPassword("NewPassword");

    } catch (const std::exception& e) {
        qDebug() << "   Caught expected error:" << e.what();
    }

    // Example 3: Create without parent group
    qDebug() << "\n3. Create without parent group (should fail validation):";
    try {
        // We can't directly test this without exposing internal constructors
        // Instead, show that the Builder methods prevent this
        qDebug() << "   Builder methods automatically prevent this scenario";
        qDebug() << "   Builder::createEntry() requires a parent group parameter";

    } catch (const std::exception& e) {
        qDebug() << "   Error:" << e.what();
    }
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "New Transaction Builder Architecture Demo";
    qDebug() << "========================================";

    demonstrateEntryTransactions();
    demonstrateGroupTransactions();
    demonstrateValidationAndErrors();

    qDebug() << "\n=== Demo Complete ===";
    qDebug() << "\nKey Benefits of New Architecture:";
    qDebug() << "• Type-safe: Cannot mix entry and group operations";
    qDebug() << "• Fluent: Clean, readable builder pattern";
    qDebug() << "• Bulk operations: Update multiple attributes at once";
    qDebug() << "• Auto-UUID: Create operations auto-generate UUIDs";
    qDebug() << "• Validation: Comprehensive error checking";
    qDebug() << "• Extensible: Easy to add new transaction types";

    return 0;
}