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

#include "Transaction.h"
#include <QJsonDocument>

Transaction::Transaction()
    : m_id(QUuid::createUuid())
    , m_type(TransactionType::UpdateEntry)
    , m_timestamp(QDateTime::currentDateTimeUtc())
{
}

Transaction::Transaction(TransactionType type, const QString& description)
    : m_id(QUuid::createUuid())
    , m_type(type)
    , m_timestamp(QDateTime::currentDateTimeUtc())
    , m_description(description)
{
}

Transaction::Transaction(const QJsonObject& json)
{
    fromJson(json);
}

void Transaction::setTarget(const QString& key, const QVariant& value)
{
    m_target[key] = QJsonValue::fromVariant(value);
}

QVariant Transaction::target(const QString& key) const
{
    return m_target.value(key).toVariant();
}

void Transaction::setChange(const QString& path, const QVariant& value)
{
    // For now, use a simple approach - in a full implementation we'd support nested paths
    QStringList parts = path.split('.');
    if (parts.size() == 1) {
        m_changes[path] = QJsonValue::fromVariant(value);
    } else {
        // Handle basic nesting like "attributes.Password"
        QString section = parts[0];
        QString key = parts.mid(1).join('.');

        QJsonObject sectionObj = m_changes[section].toObject();
        sectionObj[key] = QJsonValue::fromVariant(value);
        m_changes[section] = sectionObj;
    }
}

QVariant Transaction::change(const QString& path) const
{
    // Handle both nested and flat access patterns
    QStringList parts = path.split('.');
    if (parts.size() == 1) {
        return m_changes.value(path).toVariant();
    } else {
        // Handle basic nesting like "attributes.Password.value" -> "attributes" -> "Password.value"
        QString section = parts[0];
        QString key = parts.mid(1).join('.');

        QJsonObject sectionObj = m_changes[section].toObject();
        return sectionObj.value(key).toVariant();
    }
}

QJsonObject Transaction::toJson() const
{
    QJsonObject json;
    json["id"] = m_id.toString();
    json["type"] = typeToString(m_type);
    json["timestamp"] = m_timestamp.toString(Qt::ISODate);
    json["description"] = m_description;
    json["target"] = m_target;
    json["changes"] = m_changes;
    return json;
}

void Transaction::fromJson(const QJsonObject& json)
{
    m_id = QUuid(json["id"].toString());
    m_type = stringToType(json["type"].toString());
    m_timestamp = QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
    m_description = json["description"].toString();
    m_target = json["target"].toObject();
    m_changes = json["changes"].toObject();
}

bool Transaction::isValid() const
{
    if (m_id.isNull())
        return false;
    if (m_timestamp.isNull())
        return false;
    if (m_target.isEmpty())
        return false;

    // Type-specific validation
    switch (m_type) {
    case TransactionType::CreateEntry:
    case TransactionType::UpdateEntry:
    case TransactionType::DeleteEntry:
        return m_target.contains("entryId");
    case TransactionType::CreateGroup:
    case TransactionType::UpdateGroup:
    case TransactionType::DeleteGroup:
        return m_target.contains("groupId");
    case TransactionType::MoveEntry:
        return m_target.contains("entryId") && m_changes.contains("parentGroup");
    case TransactionType::MoveGroup:
        return m_target.contains("groupId") && m_changes.contains("parentGroup");
    default:
        return true;
    }
}

QString Transaction::validationError() const
{
    if (m_id.isNull())
        return "Invalid transaction ID";
    if (m_timestamp.isNull())
        return "Invalid timestamp";
    if (m_target.isEmpty())
        return "No target specified";

    switch (m_type) {
    case TransactionType::CreateEntry:
    case TransactionType::UpdateEntry:
    case TransactionType::DeleteEntry:
        if (!m_target.contains("entryId"))
            return "Entry ID required";
        break;
    case TransactionType::CreateGroup:
    case TransactionType::UpdateGroup:
    case TransactionType::DeleteGroup:
        if (!m_target.contains("groupId"))
            return "Group ID required";
        break;
    case TransactionType::MoveEntry:
        if (!m_target.contains("entryId"))
            return "Entry ID required";
        if (!m_changes.contains("parentGroup"))
            return "Parent group required for move";
        break;
    case TransactionType::MoveGroup:
        if (!m_target.contains("groupId"))
            return "Group ID required";
        if (!m_changes.contains("parentGroup"))
            return "Parent group required for move";
        break;
    case TransactionType::UpdateDatabase:
        // Database updates don't require specific targets
        break;
    }

    return QString();
}

void Transaction::setEntryTarget(const QUuid& entryId)
{
    setTarget("entryId", entryId.toString());
}

void Transaction::setGroupTarget(const QUuid& groupId)
{
    setTarget("groupId", groupId.toString());
}

void Transaction::setAttributeChange(const QString& key, const QString& value, bool isProtected)
{
    setChange(QString("attributes.%1.value").arg(key), value);
    setChange(QString("attributes.%1.protected").arg(key), isProtected);
}

void Transaction::setPropertyChange(const QString& property, const QVariant& value)
{
    setChange(QString("properties.%1").arg(property), value);
}

QString Transaction::typeToString(TransactionType type)
{
    switch (type) {
    case TransactionType::CreateEntry:
        return "CreateEntry";
    case TransactionType::UpdateEntry:
        return "UpdateEntry";
    case TransactionType::DeleteEntry:
        return "DeleteEntry";
    case TransactionType::MoveEntry:
        return "MoveEntry";
    case TransactionType::CreateGroup:
        return "CreateGroup";
    case TransactionType::UpdateGroup:
        return "UpdateGroup";
    case TransactionType::DeleteGroup:
        return "DeleteGroup";
    case TransactionType::MoveGroup:
        return "MoveGroup";
    case TransactionType::UpdateDatabase:
        return "UpdateDatabase";
    default:
        return "Unknown";
    }
}

TransactionType Transaction::stringToType(const QString& str)
{
    if (str == "CreateEntry")
        return TransactionType::CreateEntry;
    if (str == "UpdateEntry")
        return TransactionType::UpdateEntry;
    if (str == "DeleteEntry")
        return TransactionType::DeleteEntry;
    if (str == "MoveEntry")
        return TransactionType::MoveEntry;
    if (str == "CreateGroup")
        return TransactionType::CreateGroup;
    if (str == "UpdateGroup")
        return TransactionType::UpdateGroup;
    if (str == "DeleteGroup")
        return TransactionType::DeleteGroup;
    if (str == "MoveGroup")
        return TransactionType::MoveGroup;
    if (str == "UpdateDatabase")
        return TransactionType::UpdateDatabase;
    return TransactionType::UpdateEntry; // Default fallback
}