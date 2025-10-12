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

#ifndef KEEPASSX_ENTRYTRANSACTIONBUILDER_H
#define KEEPASSX_ENTRYTRANSACTIONBUILDER_H

#include "TransactionBuilderBase.h"
#include <QSet>
#include <QStringList>

class EntryTransactionBuilder : public TransactionBuilderBase
{
public:
    EntryTransactionBuilder(DirectTransactionType type);

    // Override base class methods to return EntryTransactionBuilder&
    EntryTransactionBuilder& withDescription(const QString& description) override;
    EntryTransactionBuilder& withIcon(int iconNumber) override;
    EntryTransactionBuilder& withIcon(const QUuid& iconUuid) override;
    EntryTransactionBuilder& withCustomData(const QString& key, const QString& value) override;

    // Entry-specific attribute methods
    EntryTransactionBuilder& withTitle(const QString& title);
    EntryTransactionBuilder& withUsername(const QString& username);
    EntryTransactionBuilder& withPassword(const QString& password);
    EntryTransactionBuilder& withUrl(const QString& url);
    EntryTransactionBuilder& withNotes(const QString& notes);
    EntryTransactionBuilder& withTags(const QString& tags);
    EntryTransactionBuilder& withTags(const QStringList& tags);

    // Entry-specific properties
    EntryTransactionBuilder& withForegroundColor(const QString& color);
    EntryTransactionBuilder& withBackgroundColor(const QString& color);
    EntryTransactionBuilder& withOverrideUrl(const QString& url);
    EntryTransactionBuilder& withAutoTypeEnabled(bool enabled);
    EntryTransactionBuilder& withAutoTypeObfuscation(int obfuscation);
    EntryTransactionBuilder& withDefaultAutoTypeSequence(const QString& sequence);
    EntryTransactionBuilder& withExpires(bool expires);
    EntryTransactionBuilder& withExpiryTime(const QDateTime& dateTime);

    // Custom attributes (bulk operations)
    EntryTransactionBuilder& withAttribute(const QString& key, const QString& value, bool isProtected = false);
    EntryTransactionBuilder& withAttributes(const QMap<QString, QString>& attributes);
    EntryTransactionBuilder& withProtectedAttributes(const QMap<QString, QString>& attributes);

    // Advanced bulk operations
    EntryTransactionBuilder& replaceAllAttributes(const QMap<QString, QString>& attributes,
                                                  const QSet<QString>& protectedKeys = QSet<QString>());
    EntryTransactionBuilder& clearAttributes(const QStringList& keys);
    EntryTransactionBuilder& clearAllCustomAttributes();

    // Target operations for create/move
    EntryTransactionBuilder& toGroup(const QUuid& groupId);

protected:
    void populateTransaction(DirectTransaction& transaction) const override;
    bool validateSpecific() const override;
    QString specificValidationError() const override;

private:
    // Entry-specific changes
    QMap<QString, QString> m_attributes;
    QSet<QString> m_protectedAttributes;
    QMap<QString, QVariant> m_entryProperties;
    QStringList m_attributesToClear;
    bool m_clearAllCustom;

    // Helper methods
    void setStandardAttribute(const QString& key, const QString& value);
    bool hasAnyChanges() const;
};

#endif // KEEPASSX_ENTRYTRANSACTIONBUILDER_H