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

#ifndef KEEPASSX_GROUPTRANSACTIONBUILDER_H
#define KEEPASSX_GROUPTRANSACTIONBUILDER_H

#include "TransactionBuilderBase.h"

class GroupTransactionBuilder : public TransactionBuilderBase
{
public:
    enum class TriState
    {
        Inherit,
        Enable,
        Disable
    };

    enum class MergeMode
    {
        Default,
        KeepNewer,
        Synchronize
    };

    GroupTransactionBuilder(DirectTransactionType type);

    // Override base class methods to return GroupTransactionBuilder&
    GroupTransactionBuilder& withDescription(const QString& description) override;
    GroupTransactionBuilder& withIcon(int iconNumber) override;
    GroupTransactionBuilder& withIcon(const QUuid& iconUuid) override;
    GroupTransactionBuilder& withCustomData(const QString& key, const QString& value) override;

    // Group-specific property methods
    GroupTransactionBuilder& withName(const QString& name);
    GroupTransactionBuilder& withNotes(const QString& notes);
    GroupTransactionBuilder& withTags(const QString& tags);
    GroupTransactionBuilder& withExpanded(bool expanded);
    GroupTransactionBuilder& withDefaultAutoTypeSequence(const QString& sequence);
    GroupTransactionBuilder& withAutoTypeEnabled(TriState enabled);
    GroupTransactionBuilder& withSearchingEnabled(TriState enabled);
    GroupTransactionBuilder& withMergeMode(MergeMode mode);

    // Time-related operations
    GroupTransactionBuilder& withExpires(bool expires);
    GroupTransactionBuilder& withExpiryTime(const QDateTime& dateTime);

    // Bulk property operations
    GroupTransactionBuilder& withProperties(const QMap<QString, QVariant>& properties);
    GroupTransactionBuilder& clearProperty(const QString& property);
    GroupTransactionBuilder& clearProperties(const QStringList& properties);

    // Target operations for create/move
    GroupTransactionBuilder& toParent(const QUuid& parentId);

protected:
    void populateTransaction(DirectTransaction& transaction) const override;
    bool validateSpecific() const override;
    QString specificValidationError() const override;

private:
    // Group-specific changes
    QMap<QString, QVariant> m_groupProperties;
    QStringList m_propertiesToClear;

    // Helper methods
    bool hasAnyChanges() const;
    QVariant triStateToVariant(TriState state) const;
    QVariant mergeModeToVariant(MergeMode mode) const;
};

#endif // KEEPASSX_GROUPTRANSACTIONBUILDER_H