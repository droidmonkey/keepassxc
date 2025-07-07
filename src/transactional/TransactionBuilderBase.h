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

#ifndef KEEPASSX_TRANSACTIONBUILDERBASE_H
#define KEEPASSX_TRANSACTIONBUILDERBASE_H

#include "DirectTransaction.h"
#include <QDateTime>
#include <QString>
#include <QUuid>
#include <QVariant>

class TransactionBuilderBase
{
public:
    TransactionBuilderBase(DirectTransactionType type);
    virtual ~TransactionBuilderBase() = default;

    // Common fluent interface methods
    virtual TransactionBuilderBase& withDescription(const QString& description);
    virtual TransactionBuilderBase& withIcon(int iconNumber);
    virtual TransactionBuilderBase& withIcon(const QUuid& iconUuid);
    virtual TransactionBuilderBase& withCustomData(const QString& key, const QString& value);

    // Build and validation
    DirectTransaction build();
    bool isValid() const;
    QString validationError() const;

protected:
    // Core transaction data
    DirectTransactionType m_type;
    QString m_description;
    QUuid m_targetId;
    QUuid m_parentId; // For create/move operations

    // Common changes
    QMap<QString, QVariant> m_commonProperties;
    QMap<QString, QString> m_customData;

    // Icon changes
    bool m_hasIconNumber;
    int m_iconNumber;
    bool m_hasIconUuid;
    QUuid m_iconUuid;

    // State tracking
    bool m_built;

    // Virtual methods for type-specific operations
    virtual void populateTransaction(DirectTransaction& transaction) const = 0;
    virtual bool validateSpecific() const = 0;
    virtual QString specificValidationError() const = 0;

    // Helper methods
    void setTargetId(const QUuid& targetId);
    void setParentId(const QUuid& parentId);
    void ensureNotBuilt(const QString& operation) const;
    bool isCreateOperation() const;
    bool isMoveOperation() const;

    // Make Builder a friend so it can access protected methods
    friend class Builder;
};

#endif // KEEPASSX_TRANSACTIONBUILDERBASE_H