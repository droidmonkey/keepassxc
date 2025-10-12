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

#ifndef KEEPASSX_TRANSACTION_H
#define KEEPASSX_TRANSACTION_H

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QUuid>
#include <QVariant>

enum class TransactionType
{
    CreateEntry,
    UpdateEntry,
    DeleteEntry,
    MoveEntry,
    CreateGroup,
    UpdateGroup,
    DeleteGroup,
    MoveGroup,
    UpdateDatabase
};

class Transaction
{
public:
    Transaction();
    Transaction(TransactionType type, const QString& description = QString());
    Transaction(const QJsonObject& json);

    // Basic properties
    QUuid id() const
    {
        return m_id;
    }
    TransactionType type() const
    {
        return m_type;
    }
    QDateTime timestamp() const
    {
        return m_timestamp;
    }
    QString description() const
    {
        return m_description;
    }

    // Target specification
    void setTarget(const QString& key, const QVariant& value);
    QVariant target(const QString& key) const;
    QJsonObject target() const
    {
        return m_target;
    }

    // Changes specification
    void setChange(const QString& path, const QVariant& value);
    QVariant change(const QString& path) const;
    QJsonObject changes() const
    {
        return m_changes;
    }

    // JSON serialization
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);

    // Validation
    bool isValid() const;
    QString validationError() const;

    // Convenience methods for common patterns
    void setEntryTarget(const QUuid& entryId);
    void setGroupTarget(const QUuid& groupId);
    void setAttributeChange(const QString& key, const QString& value, bool isProtected = false);
    void setPropertyChange(const QString& property, const QVariant& value);

private:
    QUuid m_id;
    TransactionType m_type;
    QDateTime m_timestamp;
    QString m_description;
    QJsonObject m_target;
    QJsonObject m_changes;

    static QString typeToString(TransactionType type);
    static TransactionType stringToType(const QString& str);
};

#endif // KEEPASSX_TRANSACTION_H