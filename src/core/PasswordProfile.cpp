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

#include "PasswordProfile.h"

PasswordProfile::PasswordProfile()
{
}

PasswordProfile::PasswordProfile(const QString& name)
    : m_name(name)
{
}

QString PasswordProfile::name() const
{
    return m_name;
}

void PasswordProfile::setName(const QString& name)
{
    m_name = name;
}

PasswordProfile::ProfileType PasswordProfile::type() const
{
    return m_type;
}

void PasswordProfile::setType(ProfileType type)
{
    m_type = type;
}

void PasswordProfile::setPasswordSettings(int length,
                                          const PasswordGenerator::CharClasses& classes,
                                          const PasswordGenerator::GeneratorFlags& flags,
                                          const QString& customCharacterSet,
                                          const QString& excludedCharacterSet)
{
    m_type = Password;
    m_passwordLength = length;
    m_charClasses = classes;
    m_generatorFlags = flags;
    m_customCharacterSet = customCharacterSet;
    m_excludedCharacterSet = excludedCharacterSet;
}

void PasswordProfile::applyPasswordSettings(PasswordGenerator* generator) const
{
    if (m_type != Password || !generator) {
        return;
    }

    generator->setLength(m_passwordLength);
    generator->setCharClasses(m_charClasses);
    generator->setFlags(m_generatorFlags);
    generator->setCustomCharacterSet(m_customCharacterSet);
    generator->setExcludedCharacterSet(m_excludedCharacterSet);
}

void PasswordProfile::setPassphraseSettings(int wordCount,
                                            PassphraseGenerator::PassphraseWordCase wordCase,
                                            const QString& wordSeparator,
                                            const QString& wordList)
{
    m_type = Passphrase;
    m_passphraseWordCount = wordCount;
    m_wordCase = wordCase;
    m_wordSeparator = wordSeparator;
    m_wordList = wordList;
}

void PasswordProfile::applyPassphraseSettings(PassphraseGenerator* generator) const
{
    if (m_type != Passphrase || !generator) {
        return;
    }

    generator->setWordCount(m_passphraseWordCount);
    generator->setWordCase(m_wordCase);
    generator->setWordSeparator(m_wordSeparator);
    if (!m_wordList.isEmpty()) {
        generator->setWordList(m_wordList);
    }
}

QVariantMap PasswordProfile::toVariantMap() const
{
    QVariantMap map;

    map["name"] = m_name;
    map["type"] = static_cast<int>(m_type);

    if (m_type == Password) {
        map["passwordLength"] = m_passwordLength;
        map["charClasses"] = static_cast<int>(m_charClasses);
        map["generatorFlags"] = static_cast<int>(m_generatorFlags);
        map["customCharacterSet"] = m_customCharacterSet;
        map["excludedCharacterSet"] = m_excludedCharacterSet;
    } else if (m_type == Passphrase) {
        map["passphraseWordCount"] = m_passphraseWordCount;
        map["wordCase"] = static_cast<int>(m_wordCase);
        map["wordSeparator"] = m_wordSeparator;
        map["wordList"] = m_wordList;
    }

    return map;
}

PasswordProfile PasswordProfile::fromVariantMap(const QVariantMap& map)
{
    PasswordProfile profile;

    profile.m_name = map.value("name").toString();
    profile.m_type = static_cast<ProfileType>(map.value("type", Password).toInt());

    if (profile.m_type == Password) {
        profile.m_passwordLength = map.value("passwordLength", PasswordGenerator::DefaultLength).toInt();
        profile.m_charClasses = static_cast<PasswordGenerator::CharClasses>(
            map.value("charClasses", static_cast<int>(PasswordGenerator::DefaultCharset)).toInt());
        profile.m_generatorFlags = static_cast<PasswordGenerator::GeneratorFlags>(
            map.value("generatorFlags", static_cast<int>(PasswordGenerator::DefaultFlags)).toInt());
        profile.m_customCharacterSet = map.value("customCharacterSet").toString();
        profile.m_excludedCharacterSet = map.value("excludedCharacterSet").toString();
    } else if (profile.m_type == Passphrase) {
        profile.m_passphraseWordCount = map.value("passphraseWordCount", PassphraseGenerator::DefaultWordCount).toInt();
        profile.m_wordCase = static_cast<PassphraseGenerator::PassphraseWordCase>(
            map.value("wordCase", static_cast<int>(PassphraseGenerator::LOWERCASE)).toInt());
        profile.m_wordSeparator = map.value("wordSeparator", PassphraseGenerator::DefaultSeparator).toString();
        profile.m_wordList = map.value("wordList", PassphraseGenerator::DefaultWordList).toString();
    }

    return profile;
}

bool PasswordProfile::isValid() const
{
    if (m_name.isEmpty()) {
        return false;
    }

    if (m_type == Password) {
        return m_passwordLength > 0;
    } else if (m_type == Passphrase) {
        return m_passphraseWordCount > 0;
    }

    return false;
}