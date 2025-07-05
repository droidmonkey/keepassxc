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

QTEST_MAIN(TransactionalTest)
#include "TransactionalTest.moc"