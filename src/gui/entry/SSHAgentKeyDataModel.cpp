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

#include "SSHAgentKeyDataModel.h"

#ifdef WITH_XC_SSHAGENT

#include "core/Entry.h"
#include "core/EntryAttachments.h"
#include "sshagent/OpenSSHKey.h"
#include "sshagent/OpenSSHKeyGenDialog.h"
#include "sshagent/SSHAgent.h"
#include <QDialog>
#include <QWidget>
#include <QUuid>

SSHAgentKeyDataModel::SSHAgentKeyDataModel(QObject* parent)
    : QObject(parent)
{
}

SSHAgentKeyDataModel::~SSHAgentKeyDataModel() = default;

void SSHAgentKeyDataModel::loadFromEntry(Entry* entry)
{
    m_entry = entry;
    
    if (!entry) {
        clear();
        return;
    }

    // Load KeeAgent settings
    m_settings.fromEntry(entry);
    
    // Extract key information
    extractKeyFromEntry();
    updateKeyInfo();
    
    emit dataChanged();
}

void SSHAgentKeyDataModel::saveToEntry(Entry* entry)
{
    if (!entry) {
        return;
    }

    // Save KeeAgent settings to entry
    m_settings.toEntry(entry);
}

void SSHAgentKeyDataModel::clear()
{
    m_entry = nullptr;
    m_settings = KeeAgentSettings();
    
    m_keyType.clear();
    m_fingerprint.clear();
    m_publicKey.clear();
    m_comment.clear();
    m_hasValidKey = false;
    
    emit dataChanged();
    emit keyDataChanged();
}

bool SSHAgentKeyDataModel::isEnabled() const
{
    return m_settings.allowUseOfSshKey();
}

bool SSHAgentKeyDataModel::addAtDatabaseOpen() const
{
    return m_settings.addAtDatabaseOpen();
}

bool SSHAgentKeyDataModel::removeAtDatabaseClose() const
{
    return m_settings.removeAtDatabaseClose();
}

bool SSHAgentKeyDataModel::requireUserConfirmation() const
{
    return m_settings.useConfirmConstraintWhenAdding();
}

bool SSHAgentKeyDataModel::useLifetimeConstraint() const
{
    return m_settings.useLifetimeConstraintWhenAdding();
}

int SSHAgentKeyDataModel::lifetimeConstraintDuration() const
{
    return m_settings.lifetimeConstraintDuration();
}

QString SSHAgentKeyDataModel::keyType() const
{
    return m_keyType;
}

QString SSHAgentKeyDataModel::fingerprint() const
{
    return m_fingerprint;
}

QString SSHAgentKeyDataModel::publicKey() const
{
    return m_publicKey;
}

QString SSHAgentKeyDataModel::comment() const
{
    return m_comment;
}

bool SSHAgentKeyDataModel::hasValidKey() const
{
    return m_hasValidKey;
}

bool SSHAgentKeyDataModel::isKeyLoadedInAgent() const
{
    if (!m_hasValidKey || !m_entry) {
        return false;
    }

    auto agent = SSHAgent::instance();
    if (!agent) {
        return false;
    }

    OpenSSHKey key;
    // Need to cast away const since toOpenSSHKey is not const
    KeeAgentSettings& settings = const_cast<KeeAgentSettings&>(m_settings);
    if (!settings.toOpenSSHKey(m_entry, key, false)) {
        return false;
    }

    bool loaded = false;
    return agent->checkIdentity(key, loaded) && loaded;
}

void SSHAgentKeyDataModel::setEnabled(bool enabled)
{
    if (m_settings.allowUseOfSshKey() != enabled) {
        m_settings.setAllowUseOfSshKey(enabled);
        emit dataChanged();
    }
}

void SSHAgentKeyDataModel::setAddAtDatabaseOpen(bool add)
{
    if (m_settings.addAtDatabaseOpen() != add) {
        m_settings.setAddAtDatabaseOpen(add);
        emit dataChanged();
    }
}

void SSHAgentKeyDataModel::setRemoveAtDatabaseClose(bool remove)
{
    if (m_settings.removeAtDatabaseClose() != remove) {
        m_settings.setRemoveAtDatabaseClose(remove);
        emit dataChanged();
    }
}

void SSHAgentKeyDataModel::setRequireUserConfirmation(bool require)
{
    if (m_settings.useConfirmConstraintWhenAdding() != require) {
        m_settings.setUseConfirmConstraintWhenAdding(require);
        emit dataChanged();
    }
}

void SSHAgentKeyDataModel::setUseLifetimeConstraint(bool use)
{
    if (m_settings.useLifetimeConstraintWhenAdding() != use) {
        m_settings.setUseLifetimeConstraintWhenAdding(use);
        emit dataChanged();
    }
}

void SSHAgentKeyDataModel::setLifetimeConstraintDuration(int duration)
{
    if (m_settings.lifetimeConstraintDuration() != duration) {
        m_settings.setLifetimeConstraintDuration(duration);
        emit dataChanged();
    }
}

bool SSHAgentKeyDataModel::generateNewKey(const QString& type, int length, const QString& comment)
{
    if (!m_entry) {
        emit errorOccurred(tr("No entry loaded"));
        return false;
    }

    auto dialog = new OpenSSHKeyGenDialog(qobject_cast<QWidget*>(parent()));
    OpenSSHKey key;
    dialog->setKey(&key);

    bool result = false;
    if (dialog->exec() == QDialog::Accepted) {
        // Use the pattern from EditEntryWidget::generatePrivateKey
        QString keyPrefix = key.type();
        if (keyPrefix.startsWith("ecdsa")) {
            keyPrefix = "id_ecdsa";
        } else {
            keyPrefix.replace("ssh-", "id_");
        }

        QString keyName = keyPrefix;
        for (int i = 0; i < 10; i++) {
            if (i > 0) {
                keyName = keyPrefix + "." + QString::number(i);
            }

            if (!m_entry->attachments()->hasKey(keyName)) {
                m_entry->attachments()->set(keyName, key.privateKey().toUtf8());
                
                // Update settings to use the new attachment
                m_settings.setSelectedType("attachment");
                m_settings.setAttachmentName(keyName);
                
                // Reload key information
                extractKeyFromEntry();
                updateKeyInfo();
                
                emit keyDataChanged();
                emit dataChanged();
                
                result = true;
                break;
            }
        }
        
        if (!result) {
            emit errorOccurred(tr("Could not find available attachment name for key"));
        }
    }
    
    dialog->deleteLater();
    return result;
}

bool SSHAgentKeyDataModel::validateCurrentKey()
{
    return extractKeyFromEntry();
}

bool SSHAgentKeyDataModel::addKeyToAgent()
{
    if (!m_hasValidKey) {
        emit errorOccurred(tr("No valid key to add to agent"));
        return false;
    }

    auto agent = SSHAgent::instance();
    if (!agent) {
        emit errorOccurred(tr("SSH agent not available"));
        return false;
    }

    try {
        OpenSSHKey key;
        if (!m_settings.toOpenSSHKey(m_entry, key, true)) {
            emit errorOccurred(tr("Failed to load key from entry: %1").arg(m_settings.errorString()));
            return false;
        }

        if (!agent->addIdentity(key, m_settings, QUuid())) {
            emit errorOccurred(tr("Failed to add key to SSH agent: %1").arg(agent->errorString()));
            return false;
        }

        emit agentStatusChanged();
        return true;
    } catch (const std::exception& e) {
        emit errorOccurred(tr("Error adding key to agent: %1").arg(e.what()));
        return false;
    }
}

bool SSHAgentKeyDataModel::removeKeyFromAgent()
{
    if (!m_hasValidKey) {
        emit errorOccurred(tr("No valid key to remove from agent"));
        return false;
    }

    auto agent = SSHAgent::instance();
    if (!agent) {
        emit errorOccurred(tr("SSH agent not available"));
        return false;
    }

    OpenSSHKey key;
    if (!m_settings.toOpenSSHKey(m_entry, key, false)) {
        emit errorOccurred(tr("Failed to load key from entry"));
        return false;
    }

    if (!agent->removeIdentity(key)) {
        emit errorOccurred(tr("Failed to remove key from SSH agent: %1").arg(agent->errorString()));
        return false;
    }

    emit agentStatusChanged();
    return true;
}

QString SSHAgentKeyDataModel::getPublicKeyForCopy()
{
    if (m_publicKey.isEmpty()) {
        return QString();
    }

    return QString("%1 %2").arg(m_publicKey, m_comment);
}

void SSHAgentKeyDataModel::updateKeyInfo()
{
    if (!m_entry) {
        m_hasValidKey = false;
        return;
    }

    try {
        OpenSSHKey key;
        if (m_settings.toOpenSSHKey(m_entry, key, false)) {
            m_keyType = key.type();
            m_fingerprint = key.fingerprint(QCryptographicHash::Md5) + "\n" + key.fingerprint(QCryptographicHash::Sha256);
            m_publicKey = key.publicKey();
            m_comment = key.comment();
            m_hasValidKey = true;
        } else {
            m_keyType.clear();
            m_fingerprint.clear();
            m_publicKey.clear();
            m_comment.clear();
            m_hasValidKey = false;
        }
    } catch (const std::exception&) {
        m_hasValidKey = false;
    }

    emit keyDataChanged();
}

bool SSHAgentKeyDataModel::extractKeyFromEntry()
{
    if (!m_entry) {
        return false;
    }

    // The key is valid if KeeAgent settings are configured
    m_hasValidKey = m_settings.keyConfigured();
    return m_hasValidKey;
}

#endif // WITH_XC_SSHAGENT