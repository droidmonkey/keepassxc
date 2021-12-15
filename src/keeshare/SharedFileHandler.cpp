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

#include "SharedFileHandler.h"

#include "KeeShare.h"
#include "KeeShareSettings.h"
#include "SharedGroup.h"

#include "core/Database.h"
#include "core/FileWatcher.h"
#include "core/Group.h"
#include "core/Merger.h"
#include "core/Metadata.h"
#include "crypto/Random.h"
#include "format/KeePass2Reader.h"
#include "format/KeePass2Writer.h"
#include "gui/Icons.h"
#include "keys/CompositeKey.h"

#include <QBuffer>
#include <QFileInfo>
#include <QScopedPointer>
#include <botan/pubkey.h>
#include <unzip.h>
#include <zip.h>

namespace
{
    constexpr int FileWatchPeriod = 30;
    constexpr int FileWatchSize = 5;

    QByteArray readZipFile(void* uf)
    {
        QByteArray data;
        int bytes, bytesRead = 0;
        unzOpenCurrentFile(uf);
        do {
            data.resize(data.size() + 8192);
            bytes = unzReadCurrentFile(uf, data.data() + bytesRead, 8192);
            if (bytes > 0) {
                bytesRead += bytes;
            }
        } while (bytes > 0);
        unzCloseCurrentFile(uf);
        data.truncate(bytesRead);
        return data;
    }

    bool writeZipFile(void* zf, const QString& fileName, const QByteArray& data)
    {
        zipOpenNewFileInZip64(zf,
                              fileName.toLatin1().data(),
                              nullptr,
                              nullptr,
                              0,
                              nullptr,
                              0,
                              nullptr,
                              Z_DEFLATED,
                              Z_DEFAULT_COMPRESSION,
                              1);
        int pos = 0;
        do {
            auto len = qMin(data.size() - pos, 8192);
            zipWriteInFileInZip(zf, data.data() + pos, len);
            pos += len;
        } while (pos < data.size());

        zipCloseFileInZip(zf);
        return true;
    }

    bool signData(const QByteArray& data, const KeeShareSettings::Key& key, QString& signature)
    {
        if (key.key->algo_name() == "RSA") {
            try {
                Botan::PK_Signer signer(*key.key, "EMSA3(SHA-256)");
                signer.update(reinterpret_cast<const uint8_t*>(data.constData()), data.size());
                auto s = signer.signature(*randomGen()->getRng());

                auto hex = QByteArray(reinterpret_cast<char*>(s.data()), s.size()).toHex();
                signature = QString("rsa|%1").arg(QString::fromLatin1(hex));
                return true;
            } catch (std::exception& e) {
                qWarning("KeeShare: Failed to sign data: %s", e.what());
                return false;
            }
        }
        qWarning("Unsupported Public/Private key format");
        return false;
    }
} // namespace

SharedFileHandler::SharedFileHandler(QObject* parent)
    : QObject(parent)
    , m_fileWatcher(new FileWatcher(this))
{
}

SharedFileHandler::~SharedFileHandler()
{
}

bool SharedFileHandler::isValid()
{
    return !m_filePath.isEmpty();
}

void SharedFileHandler::setFilePath(const QString& filePath)
{
    if (filePath.isEmpty()) {
        m_filePath.clear();
        m_fileWatcher->stop();
    } else {
        QFileInfo info(filePath);
        m_filePath = info.absoluteFilePath();
        m_fileWatcher->start(m_filePath, FileWatchPeriod, FileWatchSize);
    }
}

QString SharedFileHandler::filePath() const
{
    return m_filePath;
}

bool SharedFileHandler::exportShare(const Group* group, const QSharedPointer<CompositeKey>& key, QString& error)
{
    QFile file(m_filePath);
    const bool fileOpened = file.open(QIODevice::WriteOnly);
    if (!fileOpened) {
        error = tr("Failed to open export file %1: %2").arg(m_filePath, file.errorString());
        return false;
    }

    auto targetDb = convertToDatabase(group);
    targetDb->setKey(key);

    if (m_filePath.endsWith(".kdbx.share")) {
        // Write database to memory and sign it
        QByteArray dbData, signatureData;
        QBuffer buffer;

        buffer.setBuffer(&dbData);
        buffer.open(QIODevice::WriteOnly);

        KeePass2Writer writer;
        if (!writer.writeDatabase(&buffer, targetDb.data())) {
            error = tr("Failed to serialize export database: %1").arg(writer.errorString());
            return false;
        }

        buffer.close();

        // Get Own Certificate for signing
        const auto own = KeeShare::own();
        Q_ASSERT(!own.isNull());

        // Sign the database data
        KeeShareSettings::Sign sign;
        sign.certificate = own.certificate;
        signData(dbData, own.key, sign.signature);

        signatureData = KeeShareSettings::Sign::serialize(sign).toLatin1();

        auto zf = zipOpen64(m_filePath.toLatin1().data(), 0);
        if (!zf) {
            error = tr("Failed to open export container %1 for writing").arg(m_filePath);
            return false;
        }

        writeZipFile(zf, KeeShare::signatureFileName().toLatin1().data(), signatureData);
        writeZipFile(zf, KeeShare::containerFileName().toLatin1().data(), dbData);

        zipClose(zf, nullptr);
    } else {
        QString saveError;
        if (!targetDb->saveAs(m_filePath, Database::Atomic, {}, &saveError)) {
            error = tr("Exporting database failed: %1").arg(saveError);
            return false;
        }
    }

    emit sharedFileSaved();
    return true;
}

QSharedPointer<Database> SharedFileHandler::importShare(const QSharedPointer<CompositeKey>& key, QString& error)
{
    QByteArray dbData;
    error.clear();

    auto uf = unzOpen64(m_filePath.toLatin1().constData());
    if (uf) {
        // Open zip share, extract database portion, ignore signature file
        char zipFileName[256];
        auto err = unzGoToFirstFile(uf);
        while (err == UNZ_OK) {
            unzGetCurrentFileInfo64(uf, nullptr, zipFileName, sizeof(zipFileName), nullptr, 0, nullptr, 0);
            if (QString(zipFileName).compare(KeeShare::containerFileName()) == 0) {
                dbData = readZipFile(uf);
            }
            err = unzGoToNextFile(uf);
        }
        unzClose(uf);
    } else {
        // Open KDBX file directly
        QFile file(m_filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            error = tr("Unable to open file %1: %2").arg(m_filePath, file.errorString());
            return {};
        }
        dbData = file.readAll();
    }

    QBuffer buffer(&dbData);
    buffer.open(QIODevice::ReadOnly);

    auto sourceDb = QSharedPointer<Database>::create();
    sourceDb->setKey(key);

    KeePass2Reader reader;
    if (!reader.readDatabase(&buffer, key, sourceDb.data())) {
        error = tr("Failed to read database: %1").arg(reader.errorString());
        return {};
    }

    return sourceDb;
}

QSharedPointer<Database> SharedFileHandler::convertToDatabase(const Group* group)
{
    const auto sourceDb = group->database();
    auto targetDb = QSharedPointer<Database>::create();
    targetDb->setKdf(KeePass2::uuidToKdf(KeePass2::KDF_ARGON2D));

    auto targetMetadata = targetDb->metadata();
    targetMetadata->setName(group->name());
    targetMetadata->setRecycleBinEnabled(false);

    // Clone the source group into the target and then copy custom icons as necessary
    auto targetRoot = group->clone(Entry::CloneIncludeHistory, Group::CloneIncludeEntries);
    for (const auto entry : group->entriesRecursive(false)) {
        // Copy custom icon, if necessary
        const auto iconUuid = entry->iconUuid();
        if (!iconUuid.isNull() && !targetMetadata->hasCustomIcon(iconUuid)) {
            targetMetadata->addCustomIcon(iconUuid, sourceDb->metadata()->customIcon(iconUuid));
        }
    }

    auto* obsoleteRoot = targetDb->rootGroup();
    targetDb->setRootGroup(targetRoot);
    delete obsoleteRoot;

    // TODO: need better handling of deleted objects, bound to share only
    targetDb->setDeletedObjects(sourceDb->deletedObjects());
    return targetDb;
}
