/*
 *  Copyright (C) 2019 KeePassXC Team <team@keepassxc.org>
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
#include "ShareExport.h"
#include "core/Group.h"
#include "core/Metadata.h"
#include "format/KeePass2Writer.h"
#include "gui/MessageBox.h"
#include "keeshare/KeeShare.h"
#include "keeshare/Signature.h"
#include "keys/PasswordKey.h"

#include <QBuffer>
#include <zip.h>

namespace
{
    void resolveReferenceAttributes(Entry* targetEntry, const Database* sourceDb)
    {
        for (const auto& attribute : EntryAttributes::DefaultAttributes) {
            const auto standardValue = targetEntry->attributes()->value(attribute);
            const auto type = targetEntry->placeholderType(standardValue);
            if (type != Entry::PlaceholderType::Reference) {
                // No reference to resolve
                continue;
            }
            const auto* referencedTargetEntry = targetEntry->resolveReference(standardValue);
            if (referencedTargetEntry) {
                // References is within scope, no resolving needed
                continue;
            }
            // We could do more sophisticated **** trying to point the reference to the next in-scope reference
            // but those cases with high propability constructed examples and very rare in real usage
            const auto* sourceReference = sourceDb->rootGroup()->findEntryByUuid(targetEntry->uuid());
            const auto resolvedValue = sourceReference->resolveMultiplePlaceholders(standardValue);
            targetEntry->setUpdateTimeinfo(false);
            targetEntry->attributes()->set(attribute, resolvedValue, targetEntry->attributes()->isProtected(attribute));
            targetEntry->setUpdateTimeinfo(true);
        }
    }

    Database* extractIntoDatabase(const KeeShareSettings::Reference& reference, const Group* sourceRoot)
    {
        const auto* sourceDb = sourceRoot->database();
        auto* targetDb = new Database();
        auto* targetMetadata = targetDb->metadata();
        targetMetadata->setRecycleBinEnabled(false);

        // Copy the source root as the root of the export database, memory manage the old root node
        auto* targetRoot = sourceRoot->clone(Entry::CloneNoFlags, Group::CloneNoFlags);
        const bool updateTimeinfo = targetRoot->canUpdateTimeinfo();
        targetRoot->setUpdateTimeinfo(false);
        KeeShare::setReferenceTo(targetRoot, KeeShareSettings::Reference());
        targetRoot->setUpdateTimeinfo(updateTimeinfo);
        const auto sourceEntries = sourceRoot->entriesRecursive(false);
        for (const Entry* sourceEntry : sourceEntries) {
            auto* targetEntry = sourceEntry->clone(Entry::CloneIncludeHistory);
            const bool updateTimeinfoEntry = targetEntry->canUpdateTimeinfo();
            targetEntry->setUpdateTimeinfo(false);
            targetEntry->setGroup(targetRoot);
            targetEntry->setUpdateTimeinfo(updateTimeinfoEntry);
            const auto iconUuid = targetEntry->iconUuid();
            if (!iconUuid.isNull() && !targetMetadata->hasCustomIcon(iconUuid)) {
                targetMetadata->addCustomIcon(iconUuid, sourceEntry->icon());
            }
        }

        auto key = QSharedPointer<CompositeKey>::create();
        key->addKey(QSharedPointer<PasswordKey>::create(reference.password));
        targetDb->setKey(key);

        auto* obsoleteRoot = targetDb->rootGroup();
        targetDb->setRootGroup(targetRoot);
        delete obsoleteRoot;

        targetDb->metadata()->setName(sourceRoot->name());

        // Push all deletions of the source database to the target
        // simple moving out of a share group will not trigger a deletion in the
        // target - a more elaborate mechanism may need the use of another custom
        // attribute to share unshared entries from the target db
        for (const auto& object : sourceDb->deletedObjects()) {
            targetDb->addDeletedObject(object);
        }
        for (auto* targetEntry : targetRoot->entriesRecursive(false)) {
            if (targetEntry->hasReferences()) {
                resolveReferenceAttributes(targetEntry, sourceDb);
            }
        }
        return targetDb;
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
} // namespace

ShareObserver::Result ShareExport::intoContainer(const QString& resolvedPath,
                                                 const KeeShareSettings::Reference& reference,
                                                 const Group* group)
{
    QFile file(resolvedPath);
    const bool fileOpened = file.open(QIODevice::WriteOnly);
    if (!fileOpened) {
        qWarning("Opening export file failed");
        return {resolvedPath, ShareObserver::Result::Error, file.errorString()};
    }

    QScopedPointer<Database> targetDb(extractIntoDatabase(reference, group));
    if (resolvedPath.endsWith(".kdbx.share")) {
        // Write database to memory and sign it
        QByteArray dbData, signData;
        QBuffer buffer;

        buffer.setBuffer(&dbData);
        buffer.open(QIODevice::WriteOnly);

        KeePass2Writer writer;
        if (!writer.writeDatabase(&buffer, targetDb.data())) {
            qWarning("Serializing export dabase failed: %s.", writer.errorString().toLatin1().data());
            return {reference.path, ShareObserver::Result::Error, writer.errorString()};
        }

        buffer.close();

        // Get Own Certificate for signing
        const auto own = KeeShare::own();
        Q_ASSERT(!own.isNull());

        // Sign the database data
        KeeShareSettings::Sign sign;
        Signature::create(dbData, own.key.key, sign.signature);
        sign.certificate = own.certificate;
        signData = KeeShareSettings::Sign::serialize(sign).toLatin1();

        auto zf = zipOpen64(resolvedPath.toLatin1().data(), 0);
        if (!zf) {
            return {reference.path, ShareObserver::Result::Error, ShareExport::tr("Could not write export container.")};
        }

        writeZipFile(zf, KeeShare::signatureFileName().toLatin1().data(), signData);
        writeZipFile(zf, KeeShare::containerFileName().toLatin1().data(), dbData);

        zipClose(zf, nullptr);
    } else {
        QString error;
        if (!targetDb->saveAs(resolvedPath, &error)) {
            qWarning("Exporting dabase failed: %s.", error.toLatin1().data());
            return {resolvedPath, ShareObserver::Result::Error, error};
        }
    }

    return {resolvedPath};
}
