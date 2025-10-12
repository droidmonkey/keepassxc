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

#include "EntryTransactionBuilder.h"

EntryTransactionBuilder::EntryTransactionBuilder(DirectTransactionType type)
    : TransactionBuilderBase(type)
    , m_clearAllCustom(false)
{
}

// Override base class methods to return EntryTransactionBuilder&
EntryTransactionBuilder& EntryTransactionBuilder::withDescription(const QString& description)
{
    TransactionBuilderBase::withDescription(description);
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withIcon(int iconNumber)
{
    TransactionBuilderBase::withIcon(iconNumber);
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withIcon(const QUuid& iconUuid)
{
    TransactionBuilderBase::withIcon(iconUuid);
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withCustomData(const QString& key, const QString& value)
{
    TransactionBuilderBase::withCustomData(key, value);
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withTitle(const QString& title)
{
    ensureNotBuilt("withTitle");
    setStandardAttribute("Title", title);
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withUsername(const QString& username)
{
    ensureNotBuilt("withUsername");
    setStandardAttribute("UserName", username);
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withPassword(const QString& password)
{
    ensureNotBuilt("withPassword");
    setStandardAttribute("Password", password);
    m_protectedAttributes.insert("Password");
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withUrl(const QString& url)
{
    ensureNotBuilt("withUrl");
    setStandardAttribute("URL", url);
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withNotes(const QString& notes)
{
    ensureNotBuilt("withNotes");
    setStandardAttribute("Notes", notes);
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withTags(const QString& tags)
{
    ensureNotBuilt("withTags");
    m_entryProperties["tags"] = tags;
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withTags(const QStringList& tags)
{
    ensureNotBuilt("withTags");
    m_entryProperties["tags"] = tags.join(", ");
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withForegroundColor(const QString& color)
{
    ensureNotBuilt("withForegroundColor");
    m_entryProperties["foregroundColor"] = color;
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withBackgroundColor(const QString& color)
{
    ensureNotBuilt("withBackgroundColor");
    m_entryProperties["backgroundColor"] = color;
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withOverrideUrl(const QString& url)
{
    ensureNotBuilt("withOverrideUrl");
    m_entryProperties["overrideUrl"] = url;
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withAutoTypeEnabled(bool enabled)
{
    ensureNotBuilt("withAutoTypeEnabled");
    m_entryProperties["autoTypeEnabled"] = enabled;
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withAutoTypeObfuscation(int obfuscation)
{
    ensureNotBuilt("withAutoTypeObfuscation");
    m_entryProperties["autoTypeObfuscation"] = obfuscation;
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withDefaultAutoTypeSequence(const QString& sequence)
{
    ensureNotBuilt("withDefaultAutoTypeSequence");
    m_entryProperties["defaultAutoTypeSequence"] = sequence;
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withExpires(bool expires)
{
    ensureNotBuilt("withExpires");
    m_entryProperties["expires"] = expires;
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withExpiryTime(const QDateTime& dateTime)
{
    ensureNotBuilt("withExpiryTime");
    m_entryProperties["expiryTime"] = dateTime;
    return *this;
}

EntryTransactionBuilder&
EntryTransactionBuilder::withAttribute(const QString& key, const QString& value, bool isProtected)
{
    ensureNotBuilt("withAttribute");
    m_attributes[key] = value;
    if (isProtected) {
        m_protectedAttributes.insert(key);
    } else {
        m_protectedAttributes.remove(key);
    }
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withAttributes(const QMap<QString, QString>& attributes)
{
    ensureNotBuilt("withAttributes");
    for (auto it = attributes.constBegin(); it != attributes.constEnd(); ++it) {
        m_attributes[it.key()] = it.value();
        m_protectedAttributes.remove(it.key()); // Not protected by default
    }
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::withProtectedAttributes(const QMap<QString, QString>& attributes)
{
    ensureNotBuilt("withProtectedAttributes");
    for (auto it = attributes.constBegin(); it != attributes.constEnd(); ++it) {
        m_attributes[it.key()] = it.value();
        m_protectedAttributes.insert(it.key());
    }
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::replaceAllAttributes(const QMap<QString, QString>& attributes,
                                                                       const QSet<QString>& protectedKeys)
{
    ensureNotBuilt("replaceAllAttributes");
    m_clearAllCustom = true;
    m_attributes = attributes;
    m_protectedAttributes = protectedKeys;
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::clearAttributes(const QStringList& keys)
{
    ensureNotBuilt("clearAttributes");
    m_attributesToClear.append(keys);
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::clearAllCustomAttributes()
{
    ensureNotBuilt("clearAllCustomAttributes");
    m_clearAllCustom = true;
    return *this;
}

EntryTransactionBuilder& EntryTransactionBuilder::toGroup(const QUuid& groupId)
{
    ensureNotBuilt("toGroup");
    setParentId(groupId);
    return *this;
}

void EntryTransactionBuilder::populateTransaction(DirectTransaction& transaction) const
{
    // Set entry target
    if (isCreateOperation() || isMoveOperation()) {
        transaction.setEntryTarget(m_targetId, m_parentId);
    } else {
        transaction.setEntryTarget(m_targetId);
    }

    // Populate entry changes
    EntryChanges changes;
    changes.attributes = m_attributes;

    // Set protected status for attributes
    for (const QString& key : m_attributes.keys()) {
        changes.protectedAttributes[key] = m_protectedAttributes.contains(key);
    }

    // Add entry properties and common properties
    changes.properties = m_entryProperties;
    for (auto it = m_commonProperties.constBegin(); it != m_commonProperties.constEnd(); ++it) {
        changes.properties[it.key()] = it.value();
    }

    // Add clearing instructions
    if (m_clearAllCustom) {
        changes.properties["clearAllCustomAttributes"] = true;
    }
    if (!m_attributesToClear.isEmpty()) {
        changes.properties["clearAttributes"] = m_attributesToClear;
    }

    transaction.setEntryChanges(changes);
}

bool EntryTransactionBuilder::validateSpecific() const
{
    // For update operations, must have at least one change
    if (m_type == DirectTransactionType::UpdateEntry && !hasAnyChanges()) {
        return false;
    }

    return true;
}

QString EntryTransactionBuilder::specificValidationError() const
{
    if (m_type == DirectTransactionType::UpdateEntry && !hasAnyChanges()) {
        return "Update operations require at least one change";
    }

    return QString();
}

void EntryTransactionBuilder::setStandardAttribute(const QString& key, const QString& value)
{
    m_attributes[key] = value;
}

bool EntryTransactionBuilder::hasAnyChanges() const
{
    return !m_attributes.isEmpty() || !m_entryProperties.isEmpty() || !m_commonProperties.isEmpty()
           || !m_customData.isEmpty() || !m_attributesToClear.isEmpty() || m_clearAllCustom || m_hasIconNumber
           || m_hasIconUuid;
}