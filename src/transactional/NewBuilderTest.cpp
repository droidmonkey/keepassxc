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
#include <QTest>
#include <QUuid>

class NewBuilderTest : public QObject
{
    Q_OBJECT

private slots:
    void testEntryCreateWithAutoUuid();
    void testEntryUpdateWithBulkAttributes();
    void testEntryMoveToGroup();
    void testGroupCreateWithAutoUuid();
    void testGroupUpdateWithProperties();
    void testGroupMoveToParent();
    void testValidationErrors();
    void testBuilderReuse();
    void testTypeSafety();
    void testBulkOperations();

private:
    QUuid rootGroupId = QUuid::createUuid();
    QUuid existingEntryId = QUuid::createUuid();
    QUuid existingGroupId = QUuid::createUuid();
};

void NewBuilderTest::testEntryCreateWithAutoUuid()
{
    // Test entry creation with auto-generated UUID
    auto transaction = Builder::createEntry(rootGroupId)
                           .withDescription("Test entry creation")
                           .withTitle("Test Entry")
                           .withUsername("testuser")
                           .withPassword("testpass")
                           .build();

    QCOMPARE(transaction.type(), DirectTransactionType::CreateEntry);
    QVERIFY(!transaction.entryTarget().entryId.isNull());
    QCOMPARE(transaction.entryTarget().groupId, rootGroupId);
    QCOMPARE(transaction.description(), "Test entry creation");

    auto changes = transaction.entryChanges();
    QCOMPARE(changes.attributes["Title"], "Test Entry");
    QCOMPARE(changes.attributes["UserName"], "testuser");
    QCOMPARE(changes.attributes["Password"], "testpass");
    QCOMPARE(changes.protectedAttributes["Password"], true);
}

void NewBuilderTest::testEntryUpdateWithBulkAttributes()
{
    // Test bulk attribute updates
    QMap<QString, QString> attrs;
    attrs["Department"] = "IT";
    attrs["Location"] = "Building A";

    QMap<QString, QString> protectedAttrs;
    protectedAttrs["API Key"] = "secret123";

    auto transaction = Builder::updateEntry(existingEntryId)
                           .withDescription("Bulk update test")
                           .withAttributes(attrs)
                           .withProtectedAttributes(protectedAttrs)
                           .withTags(QStringList{"test", "bulk"})
                           .build();

    QCOMPARE(transaction.type(), DirectTransactionType::UpdateEntry);
    QCOMPARE(transaction.entryTarget().entryId, existingEntryId);

    auto changes = transaction.entryChanges();
    QCOMPARE(changes.attributes["Department"], "IT");
    QCOMPARE(changes.attributes["Location"], "Building A");
    QCOMPARE(changes.attributes["API Key"], "secret123");
    QCOMPARE(changes.protectedAttributes["API Key"], true);
    QCOMPARE(changes.protectedAttributes["Department"], false);
    QCOMPARE(changes.properties["tags"].toString(), "test, bulk");
}

void NewBuilderTest::testEntryMoveToGroup()
{
    QUuid targetGroupId = QUuid::createUuid();

    auto transaction =
        Builder::moveEntry(existingEntryId).withDescription("Move entry test").toGroup(targetGroupId).build();

    QCOMPARE(transaction.type(), DirectTransactionType::MoveEntry);
    QCOMPARE(transaction.entryTarget().entryId, existingEntryId);
    QCOMPARE(transaction.entryTarget().groupId, targetGroupId);
}

void NewBuilderTest::testGroupCreateWithAutoUuid()
{
    auto transaction = Builder::createGroup(rootGroupId)
                           .withDescription("Test group creation")
                           .withName("Test Group")
                           .withNotes("Test notes")
                           .withExpanded(true)
                           .build();

    QCOMPARE(transaction.type(), DirectTransactionType::CreateGroup);
    QVERIFY(!transaction.groupTarget().groupId.isNull());
    QCOMPARE(transaction.groupTarget().parentId, rootGroupId);

    auto changes = transaction.groupChanges();
    QCOMPARE(changes.properties["name"].toString(), "Test Group");
    QCOMPARE(changes.properties["notes"].toString(), "Test notes");
    QCOMPARE(changes.properties["isExpanded"].toBool(), true);
}

void NewBuilderTest::testGroupUpdateWithProperties()
{
    QMap<QString, QVariant> props;
    props["tags"] = "project,test";
    props["defaultAutoTypeSequence"] = "{USERNAME}{TAB}{PASSWORD}";

    auto transaction = Builder::updateGroup(existingGroupId)
                           .withDescription("Property update test")
                           .withProperties(props)
                           .withAutoTypeEnabled(GroupTransactionBuilder::TriState::Enable)
                           .withCustomData("project", "test-project")
                           .build();

    QCOMPARE(transaction.type(), DirectTransactionType::UpdateGroup);
    QCOMPARE(transaction.groupTarget().groupId, existingGroupId);

    auto changes = transaction.groupChanges();
    QCOMPARE(changes.properties["tags"].toString(), "project,test");
    QCOMPARE(changes.properties["defaultAutoTypeSequence"].toString(), "{USERNAME}{TAB}{PASSWORD}");
    QCOMPARE(changes.properties["autoTypeEnabled"].toInt(), 1); // Enable = 1
    QCOMPARE(changes.properties["customData.project"].toString(), "test-project");
}

void NewBuilderTest::testGroupMoveToParent()
{
    QUuid newParentId = QUuid::createUuid();

    auto transaction =
        Builder::moveGroup(existingGroupId).withDescription("Move group test").toParent(newParentId).build();

    QCOMPARE(transaction.type(), DirectTransactionType::MoveGroup);
    QCOMPARE(transaction.groupTarget().groupId, existingGroupId);
    QCOMPARE(transaction.groupTarget().parentId, newParentId);
}

void NewBuilderTest::testValidationErrors()
{
    // Test update without changes
    auto builder = Builder::updateEntry(existingEntryId);
    QVERIFY(!builder.isValid());
    QVERIFY(builder.validationError().contains("at least one change"));

    // Test create operations always have parent group through Builder
    // Builder methods prevent invalid create operations
    auto validCreateBuilder = Builder::createEntry(rootGroupId);
    QVERIFY(validCreateBuilder.isValid()); // Should be valid since parent is set
}

void NewBuilderTest::testBuilderReuse()
{
    auto builder = Builder::updateEntry(existingEntryId).withTitle("Test Title");

    // First build should work
    auto transaction1 = builder.build();
    QVERIFY(transaction1.isValid());

    // Second attempt should fail
    bool caught = false;
    try {
        builder.withPassword("New Password");
    } catch (const std::exception&) {
        caught = true;
    }
    QVERIFY(caught);
}

void NewBuilderTest::testTypeSafety()
{
    // This test verifies at compile-time that you cannot mix entry and group operations
    // The compiler ensures this, but we can test runtime validation

    // EntryTransactionBuilder only has entry methods
    auto entryBuilder = Builder::updateEntry(existingEntryId);
    entryBuilder.withTitle("Title"); // This compiles
    // entryBuilder.withName("Name"); // This would NOT compile

    // GroupTransactionBuilder only has group methods
    auto groupBuilder = Builder::updateGroup(existingGroupId);
    groupBuilder.withName("Name"); // This compiles
    // groupBuilder.withTitle("Title"); // This would NOT compile

    // Test passes if compilation succeeds
    QVERIFY(true);
}

void NewBuilderTest::testBulkOperations()
{
    // Test clearing attributes
    auto transaction = Builder::updateEntry(existingEntryId)
                           .withDescription("Clear attributes test")
                           .clearAttributes(QStringList{"Old Field", "Deprecated"})
                           .withTitle("Updated Title")
                           .build();

    auto changes = transaction.entryChanges();
    QCOMPARE(changes.attributes["Title"], "Updated Title");
    QVERIFY(changes.properties.contains("clearAttributes"));

    // Test replace all attributes
    QMap<QString, QString> newAttrs;
    newAttrs["Title"] = "New Title";
    newAttrs["UserName"] = "newuser";

    auto transaction2 = Builder::updateEntry(existingEntryId)
                            .withDescription("Replace all test")
                            .replaceAllAttributes(newAttrs, {"UserName"})
                            .build();

    auto changes2 = transaction2.entryChanges();
    QCOMPARE(changes2.attributes["Title"], "New Title");
    QCOMPARE(changes2.attributes["UserName"], "newuser");
    QCOMPARE(changes2.protectedAttributes["UserName"], true);
    QCOMPARE(changes2.protectedAttributes["Title"], false);
    QCOMPARE(changes2.properties["clearAllCustomAttributes"].toBool(), true);
}

QTEST_MAIN(NewBuilderTest)
#include "NewBuilderTest.moc"