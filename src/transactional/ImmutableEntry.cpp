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

#include "ImmutableEntry.h"
#include <functional>

// ImmutableEntryData implementation
ImmutableEntryData::ImmutableEntryData()
    : uuid(QUuid::createUuid())
    , created(QDateTime::currentDateTimeUtc())
    , modified(QDateTime::currentDateTimeUtc())
{
}

ImmutableEntryData::ImmutableEntryData(const QUuid& uuid)
    : uuid(uuid)
    , created(QDateTime::currentDateTimeUtc())
    , modified(QDateTime::currentDateTimeUtc())
{
}

ImmutableEntryData::ImmutableEntryData(const ImmutableEntryData& other)
    : QSharedData(other)
    , uuid(other.uuid)
    , title(other.title)
    , username(other.username)
    , password(other.password)
    , url(other.url)
    , notes(other.notes)
    , created(other.created)
    , modified(other.modified)
    , expires(other.expires)
    , attributes(other.attributes)
    , protectedAttributes(other.protectedAttributes)
{
}

// ImmutableEntry implementation
ImmutableEntry::ImmutableEntry()
    : d(new ImmutableEntryData())
{
}

ImmutableEntry::ImmutableEntry(const QUuid& uuid)
    : d(new ImmutableEntryData(uuid))
{
}

ImmutableEntry::ImmutableEntry(const ImmutableEntry& other)
    : d(other.d)
{
}

ImmutableEntry& ImmutableEntry::operator=(const ImmutableEntry& other)
{
    d = other.d;
    return *this;
}

ImmutableEntry::~ImmutableEntry() = default;

// Read-only accessors
QUuid ImmutableEntry::uuid() const
{
    return d->uuid;
}

QString ImmutableEntry::title() const
{
    return d->title;
}

QString ImmutableEntry::username() const
{
    return d->username;
}

QString ImmutableEntry::password() const
{
    return d->password;
}

QString ImmutableEntry::url() const
{
    return d->url;
}

QString ImmutableEntry::notes() const
{
    return d->notes;
}

QDateTime ImmutableEntry::created() const
{
    return d->created;
}

QDateTime ImmutableEntry::modified() const
{
    return d->modified;
}

QDateTime ImmutableEntry::expires() const
{
    return d->expires;
}

bool ImmutableEntry::isExpired() const
{
    return d->expires.isValid() && d->expires < QDateTime::currentDateTimeUtc();
}

QString ImmutableEntry::attribute(const QString& key) const
{
    return d->attributes.value(key);
}

QMap<QString, QString> ImmutableEntry::attributes() const
{
    return d->attributes;
}

bool ImmutableEntry::hasAttribute(const QString& key) const
{
    return d->attributes.contains(key);
}

bool ImmutableEntry::isAttributeProtected(const QString& key) const
{
    return d->protectedAttributes.value(key, false);
}

QMap<QString, bool> ImmutableEntry::protectedAttributes() const
{
    return d->protectedAttributes;
}

// Copy-on-write modification methods
ImmutableEntry ImmutableEntry::withTitle(const QString& title) const
{
    return withModifiedData([title](ImmutableEntryData& data) {
        data.title = title;
        data.modified = QDateTime::currentDateTimeUtc();
    });
}

ImmutableEntry ImmutableEntry::withUsername(const QString& username) const
{
    return withModifiedData([username](ImmutableEntryData& data) {
        data.username = username;
        data.modified = QDateTime::currentDateTimeUtc();
    });
}

ImmutableEntry ImmutableEntry::withPassword(const QString& password) const
{
    return withModifiedData([password](ImmutableEntryData& data) {
        data.password = password;
        data.modified = QDateTime::currentDateTimeUtc();
    });
}

ImmutableEntry ImmutableEntry::withUrl(const QString& url) const
{
    return withModifiedData([url](ImmutableEntryData& data) {
        data.url = url;
        data.modified = QDateTime::currentDateTimeUtc();
    });
}

ImmutableEntry ImmutableEntry::withNotes(const QString& notes) const
{
    return withModifiedData([notes](ImmutableEntryData& data) {
        data.notes = notes;
        data.modified = QDateTime::currentDateTimeUtc();
    });
}

ImmutableEntry ImmutableEntry::withExpires(const QDateTime& expires) const
{
    return withModifiedData([expires](ImmutableEntryData& data) {
        data.expires = expires;
        data.modified = QDateTime::currentDateTimeUtc();
    });
}

ImmutableEntry ImmutableEntry::withAttribute(const QString& key, const QString& value, bool isProtected) const
{
    return withModifiedData([key, value, isProtected](ImmutableEntryData& data) {
        data.attributes[key] = value;
        data.protectedAttributes[key] = isProtected;
        data.modified = QDateTime::currentDateTimeUtc();
    });
}

ImmutableEntry ImmutableEntry::withoutAttribute(const QString& key) const
{
    return withModifiedData([key](ImmutableEntryData& data) {
        data.attributes.remove(key);
        data.protectedAttributes.remove(key);
        data.modified = QDateTime::currentDateTimeUtc();
    });
}

ImmutableEntry ImmutableEntry::withModified(const QDateTime& modified) const
{
    return withModifiedData([modified](ImmutableEntryData& data) { data.modified = modified; });
}

// Fluent interface methods that modify the current instance
ImmutableEntry& ImmutableEntry::setTitle(const QString& title)
{
    *this = withTitle(title);
    return *this;
}

ImmutableEntry& ImmutableEntry::setUsername(const QString& username)
{
    *this = withUsername(username);
    return *this;
}

ImmutableEntry& ImmutableEntry::setPassword(const QString& password)
{
    *this = withPassword(password);
    return *this;
}

ImmutableEntry& ImmutableEntry::setUrl(const QString& url)
{
    *this = withUrl(url);
    return *this;
}

ImmutableEntry& ImmutableEntry::setNotes(const QString& notes)
{
    *this = withNotes(notes);
    return *this;
}

ImmutableEntry& ImmutableEntry::setAttribute(const QString& key, const QString& value, bool isProtected)
{
    *this = withAttribute(key, value, isProtected);
    return *this;
}

// Comparison operators
bool ImmutableEntry::operator==(const ImmutableEntry& other) const
{
    return d->uuid == other.d->uuid && d->title == other.d->title && d->username == other.d->username
           && d->password == other.d->password && d->url == other.d->url && d->notes == other.d->notes
           && d->attributes == other.d->attributes;
}

bool ImmutableEntry::operator!=(const ImmutableEntry& other) const
{
    return !(*this == other);
}

// Serialization support
QMap<QString, QVariant> ImmutableEntry::toMap() const
{
    QMap<QString, QVariant> map;
    map["uuid"] = d->uuid.toString();
    map["title"] = d->title;
    map["username"] = d->username;
    map["password"] = d->password;
    map["url"] = d->url;
    map["notes"] = d->notes;
    map["created"] = d->created;
    map["modified"] = d->modified;
    map["expires"] = d->expires;

    QVariantMap attributesMap;
    for (auto it = d->attributes.begin(); it != d->attributes.end(); ++it) {
        QVariantMap attrData;
        attrData["value"] = it.value();
        attrData["protected"] = d->protectedAttributes.value(it.key(), false);
        attributesMap[it.key()] = attrData;
    }
    map["attributes"] = attributesMap;

    return map;
}

ImmutableEntry ImmutableEntry::fromMap(const QMap<QString, QVariant>& map)
{
    ImmutableEntry entry(QUuid(map.value("uuid").toString()));

    entry.d->title = map.value("title").toString();
    entry.d->username = map.value("username").toString();
    entry.d->password = map.value("password").toString();
    entry.d->url = map.value("url").toString();
    entry.d->notes = map.value("notes").toString();
    entry.d->created = map.value("created").toDateTime();
    entry.d->modified = map.value("modified").toDateTime();
    entry.d->expires = map.value("expires").toDateTime();

    QVariantMap attributesMap = map.value("attributes").toMap();
    for (auto it = attributesMap.begin(); it != attributesMap.end(); ++it) {
        QVariantMap attrData = it.value().toMap();
        entry.d->attributes[it.key()] = attrData.value("value").toString();
        entry.d->protectedAttributes[it.key()] = attrData.value("protected").toBool();
    }

    return entry;
}

// Internal helper for copy-on-write
ImmutableEntry ImmutableEntry::withModifiedData(const std::function<void(ImmutableEntryData&)>& modifier) const
{
    ImmutableEntry result(*this);
    result.d.detach(); // Ensure we have our own copy
    modifier(*result.d);
    return result;
}