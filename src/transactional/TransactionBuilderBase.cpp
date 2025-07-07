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

#include "TransactionBuilderBase.h"
#include <stdexcept>

TransactionBuilderBase::TransactionBuilderBase(DirectTransactionType type)
    : m_type(type)
    , m_hasIconNumber(false)
    , m_iconNumber(0)
    , m_hasIconUuid(false)
    , m_built(false)
{
}

TransactionBuilderBase& TransactionBuilderBase::withDescription(const QString& description)
{
    ensureNotBuilt("withDescription");
    m_description = description;
    return *this;
}

TransactionBuilderBase& TransactionBuilderBase::withIcon(int iconNumber)
{
    ensureNotBuilt("withIcon");
    m_hasIconNumber = true;
    m_iconNumber = iconNumber;
    m_hasIconUuid = false; // Clear UUID icon if set
    return *this;
}

TransactionBuilderBase& TransactionBuilderBase::withIcon(const QUuid& iconUuid)
{
    ensureNotBuilt("withIcon");
    m_hasIconUuid = true;
    m_iconUuid = iconUuid;
    m_hasIconNumber = false; // Clear number icon if set
    return *this;
}

TransactionBuilderBase& TransactionBuilderBase::withCustomData(const QString& key, const QString& value)
{
    ensureNotBuilt("withCustomData");
    m_customData[key] = value;
    return *this;
}

DirectTransaction TransactionBuilderBase::build()
{
    if (m_built) {
        throw std::runtime_error("Transaction already built");
    }

    if (!isValid()) {
        throw std::runtime_error("Cannot build invalid transaction: " + validationError().toStdString());
    }

    DirectTransaction transaction(m_type, m_description);

    // Set common properties
    if (m_hasIconNumber) {
        m_commonProperties["iconNumber"] = m_iconNumber;
    }
    if (m_hasIconUuid) {
        m_commonProperties["iconUuid"] = m_iconUuid;
    }

    // Add custom data to properties
    for (auto it = m_customData.constBegin(); it != m_customData.constEnd(); ++it) {
        m_commonProperties["customData." + it.key()] = it.value();
    }

    // Let derived class populate specific data
    populateTransaction(transaction);

    m_built = true;
    return transaction;
}

bool TransactionBuilderBase::isValid() const
{
    // Basic validation
    if (isCreateOperation() && m_parentId.isNull()) {
        return false;
    }

    if (isMoveOperation() && m_parentId.isNull()) {
        return false;
    }

    if (!isCreateOperation() && m_targetId.isNull()) {
        return false;
    }

    // Type-specific validation
    return validateSpecific();
}

QString TransactionBuilderBase::validationError() const
{
    if (isCreateOperation() && m_parentId.isNull()) {
        return "Create operations require a parent ID";
    }

    if (isMoveOperation() && m_parentId.isNull()) {
        return "Move operations require a target parent ID";
    }

    if (!isCreateOperation() && m_targetId.isNull()) {
        return "Non-create operations require a target ID";
    }

    return specificValidationError();
}

void TransactionBuilderBase::setTargetId(const QUuid& targetId)
{
    m_targetId = targetId;
}

void TransactionBuilderBase::setParentId(const QUuid& parentId)
{
    m_parentId = parentId;
}

void TransactionBuilderBase::ensureNotBuilt(const QString& operation) const
{
    if (m_built) {
        throw std::runtime_error("Cannot call " + operation.toStdString() + " after build()");
    }
}

bool TransactionBuilderBase::isCreateOperation() const
{
    return m_type == DirectTransactionType::CreateEntry || m_type == DirectTransactionType::CreateGroup;
}

bool TransactionBuilderBase::isMoveOperation() const
{
    return m_type == DirectTransactionType::MoveEntry || m_type == DirectTransactionType::MoveGroup;
}