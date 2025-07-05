/*
 *  Copyright (C) 2025 KeePassXC Team <team@keepassxc.org>
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

#ifndef KEEPASSX_SSHAGENTKEYDATAMODEL_H
#define KEEPASSX_SSHAGENTKEYDATAMODEL_H

#include "config-keepassx.h"

#ifdef WITH_XC_SSHAGENT

#include <QObject>
#include <QString>
#include <QPointer>

#include "sshagent/KeeAgentSettings.h"

class Entry;
class OpenSSHKey;

/**
 * Data model for SSH Agent key management.
 * Handles all SSH key operations separately from UI concerns.
 */
class SSHAgentKeyDataModel : public QObject
{
    Q_OBJECT

public:
    explicit SSHAgentKeyDataModel(QObject* parent = nullptr);
    ~SSHAgentKeyDataModel() override;

    /**
     * Load SSH agent settings from an entry
     */
    void loadFromEntry(Entry* entry);

    /**
     * Save SSH agent settings to an entry
     */
    void saveToEntry(Entry* entry);

    /**
     * Clear all data
     */
    void clear();

    // Getters
    bool isEnabled() const;
    bool addAtDatabaseOpen() const;
    bool removeAtDatabaseClose() const;
    bool requireUserConfirmation() const;
    bool useLifetimeConstraint() const;
    int lifetimeConstraintDuration() const;

    QString keyType() const;
    QString fingerprint() const;
    QString publicKey() const;
    QString comment() const;

    bool hasValidKey() const;
    bool isKeyLoadedInAgent() const;

    // Setters
    void setEnabled(bool enabled);
    void setAddAtDatabaseOpen(bool add);
    void setRemoveAtDatabaseClose(bool remove);
    void setRequireUserConfirmation(bool require);
    void setUseLifetimeConstraint(bool use);
    void setLifetimeConstraintDuration(int duration);

    // Key operations
    bool processGeneratedKey(const OpenSSHKey& key);
    bool validateCurrentKey();
    bool addKeyToAgent();
    bool removeKeyFromAgent();
    QString getPublicKeyForCopy();

signals:
    /**
     * Emitted when any data changes
     */
    void dataChanged();

    /**
     * Emitted when key data specifically changes
     */
    void keyDataChanged();

    /**
     * Emitted when agent status changes
     */
    void agentStatusChanged();

    /**
     * Emitted when an error occurs
     */
    void errorOccurred(const QString& message);

private:
    void updateKeyInfo();
    bool extractKeyFromEntry();

    QPointer<Entry> m_entry;
    KeeAgentSettings m_settings;
    
    // Cached key information
    QString m_keyType;
    QString m_fingerprint;
    QString m_publicKey;
    QString m_comment;
    bool m_hasValidKey = false;
};

#endif // WITH_XC_SSHAGENT

#endif // KEEPASSX_SSHAGENTKEYDATAMODEL_H