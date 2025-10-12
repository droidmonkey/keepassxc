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

#include "ImmutableEntry.h"
#include "Transaction.h"
#include "TransactionBuilder.h"
#include "TransactionManager.h"
#include <QtTest/QtTest>

class TransactionalTest : public QObject
{
    Q_OBJECT

private slots:
    void testImmutableEntry();
    void testCopyOnWrite();
    void testFluentInterface();
    void testTransaction();
    void testTransactionSerialization();
    void testTransactionBuilder();
    void testTransactionBuilderValidation();

private:
};

void TransactionalTest::testImmutableEntry()
{
    // Test basic immutable entry functionality
    ImmutableEntry entry;

    QVERIFY(!entry.uuid().isNull());
    QVERIFY(entry.title().isEmpty());
    QVERIFY(entry.username().isEmpty());

    // Test that modifications return new instances
    ImmutableEntry entry2 = entry.withTitle("Test Entry");

    QVERIFY(entry.title().isEmpty()); // Original unchanged
    QCOMPARE(entry2.title(), QString("Test Entry")); // New instance has change
    QCOMPARE(entry.uuid(), entry2.uuid()); // Same UUID
}

void TransactionalTest::testCopyOnWrite()
{
    ImmutableEntry original;
    ImmutableEntry copy = original;

    // Verify they share data initially
    QCOMPARE(original.uuid(), copy.uuid());

    // Modify the copy
    ImmutableEntry modified = copy.withTitle("Modified");

    // Original should be unchanged
    QVERIFY(original.title().isEmpty());
    QCOMPARE(modified.title(), QString("Modified"));

    // They should still have the same UUID
    QCOMPARE(original.uuid(), modified.uuid());
}

void TransactionalTest::testFluentInterface()
{
    ImmutableEntry entry;

    // Test fluent interface chaining
    ImmutableEntry configured = entry.withTitle("GitHub Account")
                                    .withUsername("john.doe@example.com")
                                    .withPassword("SecretPassword123")
                                    .withUrl("https://github.com")
                                    .withAttribute("Notes", "Work account", false);

    QCOMPARE(configured.title(), QString("GitHub Account"));
    QCOMPARE(configured.username(), QString("john.doe@example.com"));
    QCOMPARE(configured.password(), QString("SecretPassword123"));
    QCOMPARE(configured.url(), QString("https://github.com"));
    QCOMPARE(configured.attribute("Notes"), QString("Work account"));
    QVERIFY(!configured.isAttributeProtected("Notes"));
}

void TransactionalTest::testTransaction()
{
    Transaction transaction(TransactionType::UpdateEntry, "Update GitHub password");

    QVERIFY(!transaction.id().isNull());
    QCOMPARE(transaction.type(), TransactionType::UpdateEntry);
    QCOMPARE(transaction.description(), QString("Update GitHub password"));

    // Set target and changes
    transaction.setEntryTarget(QUuid::createUuid());
    transaction.setAttributeChange("Password", "NewPassword123", true);
    transaction.setPropertyChange("modified", QDateTime::currentDateTimeUtc());

    QVERIFY(transaction.isValid());
    QVERIFY(transaction.validationError().isEmpty());
}

void TransactionalTest::testTransactionSerialization()
{
    Transaction original(TransactionType::UpdateEntry, "Test transaction");
    QUuid entryId = QUuid::createUuid();

    original.setEntryTarget(entryId);
    original.setAttributeChange("Password", "TestPassword", true);
    original.setPropertyChange("title", "New Title");

    // Serialize to JSON
    QJsonObject json = original.toJson();

    // Deserialize from JSON
    Transaction deserialized(json);

    // Verify round-trip
    QCOMPARE(deserialized.id(), original.id());
    QCOMPARE(deserialized.type(), original.type());
    QCOMPARE(deserialized.description(), original.description());
    QCOMPARE(deserialized.target("entryId").toString(), entryId.toString());
    QCOMPARE(deserialized.change("attributes.Password.value").toString(), QString("TestPassword"));
    QCOMPARE(deserialized.change("attributes.Password.protected").toBool(), true);
    QCOMPARE(deserialized.change("properties.title").toString(), QString("New Title"));
}

void TransactionalTest::testTransactionBuilder()
{
    auto entryId = QUuid::createUuid();
    auto groupId = QUuid::createUuid();

    // Test basic entry update builder
    auto transaction = TransactionBuilder::forEntry(entryId)
                           .withDescription("Test entry update")
                           .updateEntry()
                           .withTitle("Test Title")
                           .withPassword("TestPassword123")
                           .withUrl("https://example.com")
                           .build();

    QVERIFY(transaction.isValid());
    QCOMPARE(transaction.type(), DirectTransactionType::UpdateEntry);
    QCOMPARE(transaction.description(), QString("Test entry update"));
    QCOMPARE(transaction.entryTarget().entryId, entryId);
    QCOMPARE(transaction.entryChanges().attributes["Title"], QString("Test Title"));
    QCOMPARE(transaction.entryChanges().attributes["Password"], QString("TestPassword123"));
    QCOMPARE(transaction.entryChanges().protectedAttributes["Password"], true);
    QCOMPARE(transaction.entryChanges().attributes["URL"], QString("https://example.com"));

    // Test group update builder
    auto groupTransaction = TransactionBuilder::forGroup(groupId)
                                .withDescription("Test group update")
                                .updateGroup()
                                .withName("Test Group")
                                .withIcon(5)
                                .build();

    QVERIFY(groupTransaction.isValid());
    QCOMPARE(groupTransaction.type(), DirectTransactionType::UpdateGroup);
    QCOMPARE(groupTransaction.description(), QString("Test group update"));
    QCOMPARE(groupTransaction.groupTarget().groupId, groupId);
    QCOMPARE(groupTransaction.groupChanges().properties["Name"].toString(), QString("Test Group"));
    QCOMPARE(groupTransaction.groupChanges().properties["IconIndex"].toInt(), 5);

    // Test create entry builder
    auto createTransaction = TransactionBuilder::forEntry(QUuid::createUuid())
                                 .withDescription("Create new entry")
                                 .createEntry(groupId)
                                 .withTitle("New Entry")
                                 .build();

    QVERIFY(createTransaction.isValid());
    QCOMPARE(createTransaction.type(), DirectTransactionType::CreateEntry);
    QCOMPARE(createTransaction.entryTarget().groupId, groupId);
    QCOMPARE(createTransaction.entryChanges().attributes["Title"], QString("New Entry"));
}

void TransactionalTest::testTransactionBuilderValidation()
{
    auto entryId = QUuid::createUuid();

    // Test valid transaction
    auto validBuilder =
        TransactionBuilder::forEntry(entryId).withDescription("Valid update").updateEntry().withTitle("Valid Title");

    QVERIFY(validBuilder.isValid());
    QVERIFY(validBuilder.validationError().isEmpty());

    // Test invalid transaction - no operation
    auto invalidBuilder1 =
        TransactionBuilder::forEntry(entryId).withDescription("Missing operation").withTitle("Some title");

    QVERIFY(!invalidBuilder1.isValid());
    QVERIFY(invalidBuilder1.validationError().contains("No operation specified"));

    // Test invalid transaction - update without changes
    auto invalidBuilder2 =
        TransactionBuilder::forEntry(entryId).withDescription("Update without changes").updateEntry();

    QVERIFY(!invalidBuilder2.isValid());
    QVERIFY(invalidBuilder2.validationError().contains("Update operations require at least one change"));

    // Test invalid transaction - create without target
    auto invalidBuilder3 = TransactionBuilder::forEntry(QUuid::createUuid())
                               .withDescription("Create without target")
                               .createEntry(QUuid()); // Null UUID

    QVERIFY(!invalidBuilder3.isValid());
    QVERIFY(invalidBuilder3.validationError().contains("Target group ID is required"));
}

QTEST_MAIN(TransactionalTest)
#include "TransactionalTest.moc"