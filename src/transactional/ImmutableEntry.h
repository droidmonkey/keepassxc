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

#ifndef KEEPASSX_IMMUTABLE_ENTRY_H
#define KEEPASSX_IMMUTABLE_ENTRY_H

#include <QDateTime>
#include <QMap>
#include <QSharedDataPointer>
#include <QString>
#include <QUuid>
#include <QVariant>
#include <functional>

class ImmutableEntryData;

/**
 * Immutable Entry class implementing copy-on-write semantics.
 * All modifications return new instances rather than modifying in place.
 */
class ImmutableEntry
{
public:
    ImmutableEntry();
    ImmutableEntry(const QUuid& uuid);
    ImmutableEntry(const ImmutableEntry& other);
    ImmutableEntry& operator=(const ImmutableEntry& other);
    ~ImmutableEntry();

    // Basic properties (read-only)
    QUuid uuid() const;
    QString title() const;
    QString username() const;
    QString password() const;
    QString url() const;
    QString notes() const;
    QDateTime created() const;
    QDateTime modified() const;
    QDateTime expires() const;
    bool isExpired() const;

    // Attributes access
    QString attribute(const QString& key) const;
    QMap<QString, QString> attributes() const;
    bool hasAttribute(const QString& key) const;

    // Protected attributes
    bool isAttributeProtected(const QString& key) const;
    QMap<QString, bool> protectedAttributes() const;

    // Copy-on-write modification methods (return new instances)
    ImmutableEntry withTitle(const QString& title) const;
    ImmutableEntry withUsername(const QString& username) const;
    ImmutableEntry withPassword(const QString& password) const;
    ImmutableEntry withUrl(const QString& url) const;
    ImmutableEntry withNotes(const QString& notes) const;
    ImmutableEntry withExpires(const QDateTime& expires) const;
    ImmutableEntry withAttribute(const QString& key, const QString& value, bool isProtected = false) const;
    ImmutableEntry withoutAttribute(const QString& key) const;
    ImmutableEntry withModified(const QDateTime& modified = QDateTime::currentDateTimeUtc()) const;

    // Fluent interface support for chaining
    ImmutableEntry& setTitle(const QString& title);
    ImmutableEntry& setUsername(const QString& username);
    ImmutableEntry& setPassword(const QString& password);
    ImmutableEntry& setUrl(const QString& url);
    ImmutableEntry& setNotes(const QString& notes);
    ImmutableEntry& setAttribute(const QString& key, const QString& value, bool isProtected = false);

    // Comparison
    bool operator==(const ImmutableEntry& other) const;
    bool operator!=(const ImmutableEntry& other) const;

    // Serialization support
    QMap<QString, QVariant> toMap() const;
    static ImmutableEntry fromMap(const QMap<QString, QVariant>& map);

private:
    QSharedDataPointer<ImmutableEntryData> d;

    // Internal helper for copy-on-write
    ImmutableEntry withModifiedData(const std::function<void(ImmutableEntryData&)>& modifier) const;
};

/**
 * Internal data class for ImmutableEntry using implicit sharing
 */
class ImmutableEntryData : public QSharedData
{
public:
    ImmutableEntryData();
    ImmutableEntryData(const QUuid& uuid);
    ImmutableEntryData(const ImmutableEntryData& other);

    QUuid uuid;
    QString title;
    QString username;
    QString password;
    QString url;
    QString notes;
    QDateTime created;
    QDateTime modified;
    QDateTime expires;

    QMap<QString, QString> attributes;
    QMap<QString, bool> protectedAttributes;
};

#endif // KEEPASSX_IMMUTABLE_ENTRY_H