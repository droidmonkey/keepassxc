/*
 *  Copyright (C) 2021 KeePassXC Team <team@keepassxc.org>
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

#include "SharedGroup.h"

#include "SharedFileHandler.h"
#include "core/Group.h"
#include "core/Merger.h"
#include "keys/CompositeKey.h"

#include <QXmlStreamReader>

namespace
{
    const QString KeeShare_Legacy_Settings("KeeShare/Reference");
    const QString KeeShare_Type_Setting("KeeShare_Type");
    const QString KeeShare_Path_Setting("KeeShare_Path");
    const QString KeeShare_Credential_Setting("KeeShare_Credential");
} // namespace

SharedGroup::SharedGroup(Group* group)
    : QObject()
    , m_fileHandler(new SharedFileHandler(this))
    , m_group(group)
{
    Q_ASSERT(group);
    readSettings();

    // TODO: Need to figure out a better way to scope modified signals to a parent group
    connect(group->database(), &Database::databaseSaved, this, &SharedGroup::sync);
    connect(group, &Group::destroyed, this, &SharedGroup::reset);
    connect(m_fileHandler.get(), &SharedFileHandler::sharedFileChanged, this, &SharedGroup::sync);
}

SharedGroup::~SharedGroup()
{
}

bool SharedGroup::isValid()
{
    return m_group && m_fileHandler->isValid() && !m_shareKey->isEmpty();
}

SharedGroup::ShareType SharedGroup::shareType()
{
    return m_shareType;
}

const Group* SharedGroup::group()
{
    return m_group;
}

const Database* SharedGroup::database()
{
    return m_group->database();
}

bool SharedGroup::isShared(const Group* group)
{
    auto customData = group->customData();
    return customData->contains(KeeShare_Legacy_Settings) || customData->contains(KeeShare_Path_Setting);
}

void SharedGroup::sync()
{
    if (!isValid() || m_inSync) {
        return;
    }

    // Prevent re-entrance while performing a sync
    m_inSync = true;

    bool modified = false;
    QString error;
    if (m_shareType & Import) {
        auto importDB = m_fileHandler->importShare(m_shareKey, error);
        if (!importDB) {
            emit postMessage(Error, error);
            m_inSync = false;
            return;
        }

        m_group->setEmitModified(false);

        Merger merger(importDB->rootGroup(), m_group);
        merger.setForcedMergeMode(Group::Synchronize);
        auto changelist = merger.merge();

        m_group->setEmitModified(true);

        for (const auto& line : changelist) {
            modified = true;
            qInfo("%s", line.toLatin1().constData());
        }
    }

    if (m_shareType & Export) {
        if (!m_fileHandler->exportShare(m_group, m_shareKey, error)) {
            emit postMessage(Error, error);
        }
    }

    if (modified) {
        m_group->database()->markAsModified();
    }

    m_inSync = false;
}

void SharedGroup::reset()
{
    m_shareType = Inactive;
    m_shareKey.reset(new CompositeKey());
    m_fileHandler->setFilePath({});
}

void SharedGroup::readSettings()
{
    reset();

    auto customData = m_group->customData();
    if (customData->contains(KeeShare_Legacy_Settings)) {
        readLegacySettings();
    } else {
        auto type = customData->value(KeeShare_Type_Setting).toInt();
        if (type >= Inactive && type <= Synchronize) {
            m_shareType = static_cast<ShareTypeFlag>(type);
        } else {
            m_shareType = Inactive;
        }

        m_fileHandler->setFilePath(customData->value(KeeShare_Path_Setting));

        auto keyData = QByteArray::fromBase64(customData->value(KeeShare_Credential_Setting).toLatin1());
        m_shareKey->deserialize(keyData);
    }
}

void SharedGroup::readLegacySettings()
{
    const auto data = m_group->customData()->value(KeeShare_Legacy_Settings);
    const auto serialized = QString::fromUtf8(QByteArray::fromBase64(data.toLatin1()));

    QXmlStreamReader reader(serialized);
    if (!reader.readNextStartElement() || reader.qualifiedName() != "KeeShare") {
        return;
    }

    while (!reader.error() && reader.readNextStartElement()) {
        if (reader.name() == "Type") {
            while (reader.readNextStartElement()) {
                if (reader.name() == "Import") {
                    m_shareType |= Import;
                    reader.skipCurrentElement();
                } else if (reader.name() == "Export") {
                    m_shareType |= Export;
                    reader.skipCurrentElement();
                } else {
                    break;
                }
            }
        } else if (reader.name() == "Group") {
            // Ignore, we already know the group
            reader.skipCurrentElement();
        } else if (reader.name() == "Path") {
            auto path = QString::fromUtf8(QByteArray::fromBase64(reader.readElementText().toLatin1()));
            m_fileHandler->setFilePath(path);
        } else if (reader.name() == "Password") {
            auto password = QString::fromUtf8(QByteArray::fromBase64(reader.readElementText().toLatin1()));
            auto key = QSharedPointer<PasswordKey>::create(password);
            m_shareKey->addKey(key);
        } else {
            qWarning("KeeShare: Unknown share setting %s", qPrintable(reader.name().toString()));
            reader.skipCurrentElement();
        }
    }
}

void SharedGroup::writeSettings()
{
    m_group->setEmitModified(false);

    auto customData = m_group->customData();
    // Remove legacy settings
    customData->remove(KeeShare_Legacy_Settings);

    if (isValid()) {
        // Write out our share settings
        customData->set(KeeShare_Type_Setting, QString::number(m_shareType));
        customData->set(KeeShare_Path_Setting, m_fileHandler->filePath());
        customData->set(KeeShare_Credential_Setting, m_shareKey->serialize().toBase64());
    } else {
        customData->remove(KeeShare_Type_Setting);
        customData->remove(KeeShare_Path_Setting);
        customData->remove(KeeShare_Credential_Setting);
    }

    m_group->setEmitModified(true);
    m_group->database()->markAsModified();
}
