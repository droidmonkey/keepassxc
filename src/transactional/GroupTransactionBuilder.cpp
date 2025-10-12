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

#include "GroupTransactionBuilder.h"

GroupTransactionBuilder::GroupTransactionBuilder(DirectTransactionType type)
    : TransactionBuilderBase(type)
{
}

// Override base class methods to return GroupTransactionBuilder&
GroupTransactionBuilder& GroupTransactionBuilder::withDescription(const QString& description)
{
    TransactionBuilderBase::withDescription(description);
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::withIcon(int iconNumber)
{
    TransactionBuilderBase::withIcon(iconNumber);
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::withIcon(const QUuid& iconUuid)
{
    TransactionBuilderBase::withIcon(iconUuid);
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::withCustomData(const QString& key, const QString& value)
{
    TransactionBuilderBase::withCustomData(key, value);
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::withName(const QString& name)
{
    ensureNotBuilt("withName");
    m_groupProperties["name"] = name;
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::withNotes(const QString& notes)
{
    ensureNotBuilt("withNotes");
    m_groupProperties["notes"] = notes;
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::withTags(const QString& tags)
{
    ensureNotBuilt("withTags");
    m_groupProperties["tags"] = tags;
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::withExpanded(bool expanded)
{
    ensureNotBuilt("withExpanded");
    m_groupProperties["isExpanded"] = expanded;
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::withDefaultAutoTypeSequence(const QString& sequence)
{
    ensureNotBuilt("withDefaultAutoTypeSequence");
    m_groupProperties["defaultAutoTypeSequence"] = sequence;
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::withAutoTypeEnabled(TriState enabled)
{
    ensureNotBuilt("withAutoTypeEnabled");
    m_groupProperties["autoTypeEnabled"] = triStateToVariant(enabled);
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::withSearchingEnabled(TriState enabled)
{
    ensureNotBuilt("withSearchingEnabled");
    m_groupProperties["searchingEnabled"] = triStateToVariant(enabled);
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::withMergeMode(MergeMode mode)
{
    ensureNotBuilt("withMergeMode");
    m_groupProperties["mergeMode"] = mergeModeToVariant(mode);
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::withExpires(bool expires)
{
    ensureNotBuilt("withExpires");
    m_groupProperties["expires"] = expires;
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::withExpiryTime(const QDateTime& dateTime)
{
    ensureNotBuilt("withExpiryTime");
    m_groupProperties["expiryTime"] = dateTime;
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::withProperties(const QMap<QString, QVariant>& properties)
{
    ensureNotBuilt("withProperties");
    for (auto it = properties.constBegin(); it != properties.constEnd(); ++it) {
        m_groupProperties[it.key()] = it.value();
    }
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::clearProperty(const QString& property)
{
    ensureNotBuilt("clearProperty");
    m_propertiesToClear.append(property);
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::clearProperties(const QStringList& properties)
{
    ensureNotBuilt("clearProperties");
    m_propertiesToClear.append(properties);
    return *this;
}

GroupTransactionBuilder& GroupTransactionBuilder::toParent(const QUuid& parentId)
{
    ensureNotBuilt("toParent");
    setParentId(parentId);
    return *this;
}

void GroupTransactionBuilder::populateTransaction(DirectTransaction& transaction) const
{
    // Set group target
    if (isCreateOperation() || isMoveOperation()) {
        transaction.setGroupTarget(m_targetId, m_parentId);
    } else {
        transaction.setGroupTarget(m_targetId);
    }

    // Populate group changes
    GroupChanges changes;
    changes.properties = m_groupProperties;

    // Add common properties
    for (auto it = m_commonProperties.constBegin(); it != m_commonProperties.constEnd(); ++it) {
        changes.properties[it.key()] = it.value();
    }

    // Add clearing instructions
    if (!m_propertiesToClear.isEmpty()) {
        changes.properties["clearProperties"] = m_propertiesToClear;
    }

    transaction.setGroupChanges(changes);
}

bool GroupTransactionBuilder::validateSpecific() const
{
    // For update operations, must have at least one change
    if (m_type == DirectTransactionType::UpdateGroup && !hasAnyChanges()) {
        return false;
    }

    return true;
}

QString GroupTransactionBuilder::specificValidationError() const
{
    if (m_type == DirectTransactionType::UpdateGroup && !hasAnyChanges()) {
        return "Update operations require at least one change";
    }

    return QString();
}

bool GroupTransactionBuilder::hasAnyChanges() const
{
    return !m_groupProperties.isEmpty() || !m_commonProperties.isEmpty() || !m_customData.isEmpty()
           || !m_propertiesToClear.isEmpty() || m_hasIconNumber || m_hasIconUuid;
}

QVariant GroupTransactionBuilder::triStateToVariant(TriState state) const
{
    switch (state) {
    case TriState::Inherit:
        return 0;
    case TriState::Enable:
        return 1;
    case TriState::Disable:
        return 2;
    default:
        return 0;
    }
}

QVariant GroupTransactionBuilder::mergeModeToVariant(MergeMode mode) const
{
    switch (mode) {
    case MergeMode::Default:
        return 0;
    case MergeMode::KeepNewer:
        return 1;
    case MergeMode::Synchronize:
        return 2;
    default:
        return 0;
    }
}