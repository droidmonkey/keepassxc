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
#include "TransactionBuilder.h"
#include <QCoreApplication>
#include <QDebug>
#include <QUuid>

void demoBasicUsage()
{
    qDebug() << "=== TransactionBuilder Basic Usage Demo ===";
    qDebug() << "";

    auto entryId = QUuid::createUuid();
    auto groupId = QUuid::createUuid();
    auto targetGroupId = QUuid::createUuid();

    // Demo 1: Fluent entry update
    qDebug() << "Demo 1: Fluent Entry Update";
    qDebug() << "--------------------------";

    auto transaction = TransactionBuilder::forEntry(entryId)
                           .withDescription("Update GitHub account credentials")
                           .updateEntry()
                           .withTitle("GitHub Account")
                           .withUsername("john.doe@company.com")
                           .withPassword("newSecurePassword123!")
                           .withUrl("https://github.com/company")
                           .withNotes("Updated password on 2024-01-15")
                           .build();

    qDebug() << "Transaction created successfully!";
    qDebug() << "ID:" << transaction.id().toString();
    qDebug() << "Type:" << static_cast<int>(transaction.type());
    qDebug() << "Description:" << transaction.description();
    qDebug() << "Valid:" << transaction.isValid();
    qDebug() << "";

    // Demo 2: Create new entry
    qDebug() << "Demo 2: Create New Entry";
    qDebug() << "------------------------";

    auto createTransaction = TransactionBuilder::forEntry(QUuid::createUuid())
                                 .withDescription("Create new email account")
                                 .createEntry(groupId)
                                 .withTitle("Email Account")
                                 .withUsername("user@example.com")
                                 .withPassword("initialPassword456!")
                                 .build();

    qDebug() << "Create transaction built successfully!";
    qDebug() << "Description:" << createTransaction.description();
    qDebug() << "";

    // Demo 3: Move entry
    qDebug() << "Demo 3: Move Entry";
    qDebug() << "------------------";

    auto moveTransaction = TransactionBuilder::forEntry(entryId)
                               .withDescription("Move entry to Work group")
                               .moveEntryTo(targetGroupId)
                               .build();

    qDebug() << "Move transaction built successfully!";
    qDebug() << "Description:" << moveTransaction.description();
    qDebug() << "";

    // Demo 4: Group operations
    qDebug() << "Demo 4: Group Operations";
    qDebug() << "-----------------------";

    auto groupTransaction = TransactionBuilder::forGroup(groupId)
                                .withDescription("Update group properties")
                                .updateGroup()
                                .withName("Work Accounts")
                                .withIcon(42)
                                .withGroupProperty("IsExpanded", true)
                                .build();

    qDebug() << "Group transaction built successfully!";
    qDebug() << "Description:" << groupTransaction.description();
    qDebug() << "";

    // Demo 5: Database operations
    qDebug() << "Demo 5: Database Operations";
    qDebug() << "---------------------------";

    auto dbTransaction = TransactionBuilder::forDatabase()
                             .withDescription("Update database settings")
                             .updateDatabase()
                             .withDatabaseProperty("Name", "My Password Database")
                             .withDatabaseProperty("Description", "Personal passwords")
                             .build();

    qDebug() << "Database transaction built successfully!";
    qDebug() << "Description:" << dbTransaction.description();
    qDebug() << "";
}

void demoErrorHandling()
{
    qDebug() << "=== TransactionBuilder Error Handling Demo ===";
    qDebug() << "";

    // Demo 1: Mixing entry and group operations
    qDebug() << "Demo 1: Mixing Entry and Group Operations";
    qDebug() << "-------------------------------------------";

    auto entryId = QUuid::createUuid();
    auto builder = TransactionBuilder::forEntry(entryId)
                       .withDescription("Mixed operations")
                       .withName("Group name") // This should be ignored for entry builders
                       .updateEntry()
                       .withTitle("Entry title");

    qDebug() << "Builder is valid:" << builder.isValid();
    qDebug() << "Validation error:" << (builder.validationError().isEmpty() ? "None" : builder.validationError());
    qDebug() << "";

    // Demo 2: Missing operation
    qDebug() << "Demo 2: Missing Operation";
    qDebug() << "-------------------------";

    auto builder2 = TransactionBuilder::forEntry(entryId).withDescription("Missing operation").withTitle("Some title");

    qDebug() << "Builder is valid:" << builder2.isValid();
    qDebug() << "Validation error:" << builder2.validationError();
    qDebug() << "";

    // Demo 3: Update without changes
    qDebug() << "Demo 3: Update Without Changes";
    qDebug() << "------------------------------";

    auto builder3 = TransactionBuilder::forEntry(entryId).withDescription("Update without changes").updateEntry();

    qDebug() << "Builder is valid:" << builder3.isValid();
    qDebug() << "Validation error:" << builder3.validationError();
    qDebug() << "";

    // Demo 4: Multiple operations
    qDebug() << "Demo 4: Multiple Operations";
    qDebug() << "---------------------------";

    auto builder4 = TransactionBuilder::forEntry(entryId)
                        .withDescription("Multiple operations")
                        .updateEntry()
                        .deleteEntry(); // This should be ignored since operation is already set

    qDebug() << "Builder is valid:" << builder4.isValid();
    qDebug() << "Validation error:" << (builder4.validationError().isEmpty() ? "None" : builder4.validationError());
    qDebug() << "";
}

void demoValidation()
{
    qDebug() << "=== TransactionBuilder Validation Demo ===";
    qDebug() << "";

    // Demo 1: Valid transaction check
    qDebug() << "Demo 1: Valid Transaction Check";
    qDebug() << "-------------------------------";

    auto entryId = QUuid::createUuid();
    auto builder =
        TransactionBuilder::forEntry(entryId).withDescription("Valid transaction").updateEntry().withTitle("New Title");

    qDebug() << "Is valid:" << builder.isValid();
    qDebug() << "Validation error:" << (builder.validationError().isEmpty() ? "None" : builder.validationError());
    qDebug() << "";

    // Demo 2: Invalid transaction check
    qDebug() << "Demo 2: Invalid Transaction Check";
    qDebug() << "---------------------------------";

    auto invalidBuilder = TransactionBuilder::forEntry(entryId)
                              .withDescription("Invalid transaction")
                              .withTitle("Some title"); // No operation set

    qDebug() << "Is valid:" << invalidBuilder.isValid();
    qDebug() << "Validation error:" << invalidBuilder.validationError();
    qDebug() << "";
}

void demoWithManager()
{
    qDebug() << "=== TransactionBuilder with Manager Demo ===";
    qDebug() << "";

    // Create a transaction manager (note: would normally need a Database instance)
    // For demo purposes, we'll just show transaction creation
    auto entryId = QUuid::createUuid();

    // Use builder to create a transaction
    auto transaction = TransactionBuilder::forEntry(entryId)
                           .withDescription("Builder demonstration")
                           .updateEntry()
                           .withTitle("Demo Entry")
                           .withPassword("demoPassword123!")
                           .build();

    qDebug() << "Transaction built successfully!";
    qDebug() << "ID:" << transaction.id().toString();
    qDebug() << "Type:" << static_cast<int>(transaction.type());
    qDebug() << "Description:" << transaction.description();
    qDebug() << "Valid:" << transaction.isValid();
    qDebug() << "";
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "TransactionBuilder Demonstration";
    qDebug() << "================================";
    qDebug() << "";

    demoBasicUsage();
    demoErrorHandling();
    demoValidation();
    demoWithManager();

    qDebug() << "All demos completed!";

    return 0;
}