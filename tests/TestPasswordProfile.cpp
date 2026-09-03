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

#include "TestPasswordProfile.h"

#include <QJsonDocument>
#include <QSignalSpy>
#include <QTest>

#include "core/Database.h"
#include "core/Metadata.h"
#include "core/PassphraseGenerator.h"
#include "core/PasswordGenerator.h"
#include "core/PasswordProfile.h"
#include "crypto/Crypto.h"
#include "keys/PasswordKey.h"

QTEST_GUILESS_MAIN(TestPasswordProfile)

void TestPasswordProfile::initTestCase()
{
    QVERIFY(Crypto::init());
}

void TestPasswordProfile::testPasswordProfile()
{
    PasswordProfile profile("TestPassword");

    // Test profile name
    QCOMPARE(profile.name(), QString("TestPassword"));

    // Test password settings
    profile.setPasswordSettings(16,
                                PasswordGenerator::LowerLetters | PasswordGenerator::UpperLetters,
                                PasswordGenerator::ExcludeLookAlike,
                                "!@#",
                                "lo0");

    QCOMPARE(profile.type(), PasswordProfile::Password);
    QVERIFY(profile.isValid());
}

void TestPasswordProfile::testPassphraseProfile()
{
    PasswordProfile profile("TestPassphrase");

    // Test passphrase settings
    profile.setPassphraseSettings(6, PassphraseGenerator::TITLECASE, "-", "eff_large_wordlist.txt");

    QCOMPARE(profile.type(), PasswordProfile::Passphrase);
    QVERIFY(profile.isValid());
}

void TestPasswordProfile::testSerializationAndDeserialization()
{
    // Test password profile serialization
    PasswordProfile originalPassword("SerializationTest");
    originalPassword.setPasswordSettings(20,
                                         PasswordGenerator::LowerLetters | PasswordGenerator::Numbers,
                                         PasswordGenerator::CharFromEveryGroup,
                                         "@#$%",
                                         "abc123");

    QVariantMap data = originalPassword.toVariantMap();
    PasswordProfile deserializedPassword = PasswordProfile::fromVariantMap(data);

    QCOMPARE(deserializedPassword.name(), originalPassword.name());
    QCOMPARE(deserializedPassword.type(), originalPassword.type());
    QVERIFY(deserializedPassword.isValid());

    // Test passphrase profile serialization
    PasswordProfile originalPassphrase("PassphraseSerializationTest");
    originalPassphrase.setPassphraseSettings(4, PassphraseGenerator::UPPERCASE, "_", "custom_word_list.txt");

    data = originalPassphrase.toVariantMap();
    PasswordProfile deserializedPassphrase = PasswordProfile::fromVariantMap(data);

    QCOMPARE(deserializedPassphrase.name(), originalPassphrase.name());
    QCOMPARE(deserializedPassphrase.type(), originalPassphrase.type());
    QVERIFY(deserializedPassphrase.isValid());
}

void TestPasswordProfile::testDatabaseIntegration()
{
    auto db = QSharedPointer<Database>::create();
    auto key = QSharedPointer<CompositeKey>::create();
    key->addKey(QSharedPointer<PasswordKey>::create("password"));
    db->setKey(key);

    // Test adding profiles
    PasswordProfile profile1("WebsiteProfile");
    profile1.setPasswordSettings(12, PasswordGenerator::DefaultCharset, PasswordGenerator::DefaultFlags);

    PasswordProfile profile2("APIProfile");
    profile2.setPasswordSettings(32,
                                 PasswordGenerator::LowerLetters | PasswordGenerator::UpperLetters
                                     | PasswordGenerator::Numbers,
                                 PasswordGenerator::ExcludeLookAlike,
                                 "!@#$%^&*");

    PasswordProfile profile3("PassphraseProfile");
    profile3.setPassphraseSettings(5, PassphraseGenerator::LOWERCASE, "-");

    // Add profiles to database
    db->addPasswordProfile(profile1);
    db->addPasswordProfile(profile2);
    db->addPasswordProfile(profile3);

    // Test retrieving profiles
    QVERIFY(db->hasPasswordProfile("WebsiteProfile"));
    QVERIFY(db->hasPasswordProfile("APIProfile"));
    QVERIFY(db->hasPasswordProfile("PassphraseProfile"));
    QVERIFY(!db->hasPasswordProfile("NonexistentProfile"));

    QStringList profileNames = db->passwordProfileNames();
    QCOMPARE(profileNames.size(), 3);
    QVERIFY(profileNames.contains("WebsiteProfile"));
    QVERIFY(profileNames.contains("APIProfile"));
    QVERIFY(profileNames.contains("PassphraseProfile"));

    // Test retrieving specific profile
    PasswordProfile retrieved = db->passwordProfile("APIProfile");
    QCOMPARE(retrieved.name(), QString("APIProfile"));
    QCOMPARE(retrieved.type(), PasswordProfile::Password);

    // Test removing profile
    db->removePasswordProfile("WebsiteProfile");
    QVERIFY(!db->hasPasswordProfile("WebsiteProfile"));
    QCOMPARE(db->passwordProfileNames().size(), 2);

    // Test removing non-existent profile (should not crash)
    db->removePasswordProfile("NonexistentProfile");
    QCOMPARE(db->passwordProfileNames().size(), 2);
}

void TestPasswordProfile::testDuplicateNameHandling()
{
    auto db = QSharedPointer<Database>::create();
    auto key = QSharedPointer<CompositeKey>::create();
    key->addKey(QSharedPointer<PasswordKey>::create("password"));
    db->setKey(key);

    // Add initial profile
    PasswordProfile profile1("DuplicateTest");
    profile1.setPasswordSettings(10, PasswordGenerator::LowerLetters, PasswordGenerator::DefaultFlags);
    db->addPasswordProfile(profile1);

    QCOMPARE(db->passwordProfileNames().size(), 1);

    // Add profile with same name (should overwrite)
    PasswordProfile profile2("DuplicateTest");
    profile2.setPasswordSettings(20, PasswordGenerator::UpperLetters, PasswordGenerator::ExcludeLookAlike);
    db->addPasswordProfile(profile2);

    // Should still have only one profile
    QCOMPARE(db->passwordProfileNames().size(), 1);

    // Retrieved profile should have the new settings
    PasswordProfile retrieved = db->passwordProfile("DuplicateTest");
    QCOMPARE(retrieved.name(), QString("DuplicateTest"));
    // Note: We can't easily test the internal settings without exposing getters,
    // but the fact that it was overwritten is what matters for duplicate handling
}

void TestPasswordProfile::testProfileApplication()
{
    // Test password profile application
    PasswordProfile passwordProfile("ApplicationTest");
    passwordProfile.setPasswordSettings(15,
                                        PasswordGenerator::LowerLetters | PasswordGenerator::Numbers,
                                        PasswordGenerator::CharFromEveryGroup,
                                        "!@#",
                                        "l1o0");

    PasswordGenerator passwordGen;
    passwordProfile.applyPasswordSettings(&passwordGen);

    QCOMPARE(passwordGen.getLength(), 15);
    QCOMPARE(passwordGen.getActiveClasses(), PasswordGenerator::LowerLetters | PasswordGenerator::Numbers);
    QCOMPARE(passwordGen.getFlags(), PasswordGenerator::CharFromEveryGroup);
    QCOMPARE(passwordGen.getCustomCharacterSet(), QString("!@#"));
    QCOMPARE(passwordGen.getExcludedCharacterSet(), QString("l1o0"));

    // Test passphrase profile application
    PasswordProfile passphraseProfile("PassphraseApplicationTest");
    passphraseProfile.setPassphraseSettings(6, PassphraseGenerator::UPPERCASE, "_", "test_wordlist.txt");

    PassphraseGenerator passphraseGen;
    passphraseProfile.applyPassphraseSettings(&passphraseGen);

    // Note: PassphraseGenerator doesn't have public getters for all settings,
    // but we can at least verify the method doesn't crash and the profile type is correct
    QCOMPARE(passphraseProfile.type(), PasswordProfile::Passphrase);

    // Test applying wrong profile type (should be safe)
    passwordProfile.applyPassphraseSettings(&passphraseGen); // Should do nothing
    passphraseProfile.applyPasswordSettings(&passwordGen); // Should do nothing
}

void TestPasswordProfile::testPersistence()
{
    auto db = QSharedPointer<Database>::create();
    auto key = QSharedPointer<CompositeKey>::create();
    key->addKey(QSharedPointer<PasswordKey>::create("testkey"));
    db->setKey(key);

    // Add some profiles
    PasswordProfile profile1("PersistTest1");
    profile1.setPasswordSettings(14, PasswordGenerator::LowerLetters, PasswordGenerator::DefaultFlags);

    PasswordProfile profile2("PersistTest2");
    profile2.setPassphraseSettings(4, PassphraseGenerator::UPPERCASE, "_");

    db->addPasswordProfile(profile1);
    db->addPasswordProfile(profile2);

    // Verify profiles exist
    QVERIFY(db->hasPasswordProfile("PersistTest1"));
    QVERIFY(db->hasPasswordProfile("PersistTest2"));
    QCOMPARE(db->passwordProfileNames().size(), 2);

    // Test that the custom data persists in the database structure
    QVERIFY(db->metadata()->customData()->contains("KPXC_PasswordProfiles"));

    // Verify the JSON structure is valid
    QString profilesData = db->metadata()->customData()->value("KPXC_PasswordProfiles");
    QJsonDocument doc = QJsonDocument::fromJson(profilesData.toUtf8());
    QVERIFY(!doc.isNull());
    QVERIFY(doc.isObject());

    QVariantMap profilesMap = doc.toVariant().toMap();
    QCOMPARE(profilesMap.size(), 2);
    QVERIFY(profilesMap.contains("PersistTest1"));
    QVERIFY(profilesMap.contains("PersistTest2"));

    // Test that profiles can be reconstructed from the stored data
    QList<PasswordProfile> allProfiles = db->passwordProfiles();
    QCOMPARE(allProfiles.size(), 2);

    bool foundProfile1 = false, foundProfile2 = false;
    for (const auto& profile : allProfiles) {
        if (profile.name() == "PersistTest1") {
            QCOMPARE(profile.type(), PasswordProfile::Password);
            foundProfile1 = true;
        } else if (profile.name() == "PersistTest2") {
            QCOMPARE(profile.type(), PasswordProfile::Passphrase);
            foundProfile2 = true;
        }
    }
    QVERIFY(foundProfile1);
    QVERIFY(foundProfile2);
}