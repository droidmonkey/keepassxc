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

#include "SSHAgentPageController.h"

#ifdef WITH_XC_SSHAGENT

#include "SSHAgentKeyDataModel.h"
#include "SSHAgentPageWidget.h"
#include "core/Database.h"
#include "gui/Clipboard.h"
#include "gui/MessageBox.h"
#include "sshagent/OpenSSHKey.h"
#include "sshagent/OpenSSHKeyGenDialog.h"

SSHAgentPageController::SSHAgentPageController(QObject* parent)
    : EntryPageController(parent)
    , m_widget(new SSHAgentPageWidget())
    , m_dataModel(new SSHAgentKeyDataModel(this))
{
    m_widget->setDataModel(m_dataModel);
    connectSignals();
}

SSHAgentPageController::~SSHAgentPageController() = default;

void SSHAgentPageController::loadEntry(Entry* entry)
{
    m_entry = entry;
    m_dataModel->loadFromEntry(entry);
    updateUI();
}

bool SSHAgentPageController::saveEntry(Entry* entry)
{
    if (!entry) {
        return false;
    }

    m_dataModel->saveToEntry(entry);
    return true;
}

bool SSHAgentPageController::validateInput()
{
    // SSH Agent settings are generally always valid
    // The data model handles validation internally
    return true;
}

void SSHAgentPageController::clear()
{
    m_entry = nullptr;
    m_dataModel->clear();
    updateUI();
}

QWidget* SSHAgentPageController::widget()
{
    return m_widget;
}

QString SSHAgentPageController::displayName() const
{
    return tr("SSH Agent");
}

bool SSHAgentPageController::isEnabled(Database* database) const
{
    Q_UNUSED(database)
    // SSH Agent is always available when compiled with support
    return true;
}

void SSHAgentPageController::onAddKeyToAgent()
{
    if (!m_dataModel->addKeyToAgent()) {
        // Error message already emitted by data model
        return;
    }

    updateAgentStatus();
}

void SSHAgentPageController::onRemoveKeyFromAgent()
{
    if (!m_dataModel->removeKeyFromAgent()) {
        // Error message already emitted by data model
        return;
    }

    updateAgentStatus();
}

void SSHAgentPageController::onGenerateKey()
{
    if (!m_entry) {
        emit errorOccurred(tr("No entry loaded"));
        return;
    }

    auto dialog = new OpenSSHKeyGenDialog(m_widget);
    OpenSSHKey key;
    dialog->setKey(&key);

    if (dialog->exec()) {
        // Process the generated key - delegate to data model
        m_dataModel->processGeneratedKey(key);
        updateUI();
    }
    
    dialog->deleteLater();
}

void SSHAgentPageController::onCopyPublicKey()
{
    QString publicKey = m_dataModel->getPublicKeyForCopy();
    if (!publicKey.isEmpty()) {
        clipboard()->setText(publicKey);
    }
}

void SSHAgentPageController::onKeyDataChanged()
{
    updateUI();
    emit dataChanged();
}

void SSHAgentPageController::updateUI()
{
    if (m_widget) {
        m_widget->updateFromModel();
    }
}

void SSHAgentPageController::connectSignals()
{
    // Connect widget signals
    if (m_widget) {
        connect(m_widget, &SSHAgentPageWidget::addKeyToAgentRequested,
                this, &SSHAgentPageController::onAddKeyToAgent);
        connect(m_widget, &SSHAgentPageWidget::removeKeyFromAgentRequested,
                this, &SSHAgentPageController::onRemoveKeyFromAgent);
        connect(m_widget, &SSHAgentPageWidget::generateKeyRequested,
                this, &SSHAgentPageController::onGenerateKey);
        connect(m_widget, &SSHAgentPageWidget::copyPublicKeyRequested,
                this, &SSHAgentPageController::onCopyPublicKey);
    }

    // Connect data model signals
    if (m_dataModel) {
        connect(m_dataModel, &SSHAgentKeyDataModel::dataChanged,
                this, &SSHAgentPageController::onKeyDataChanged);
        connect(m_dataModel, &SSHAgentKeyDataModel::errorOccurred,
                this, &SSHAgentPageController::errorOccurred);
    }
}

void SSHAgentPageController::updateAgentStatus()
{
    // Force update of agent status
    if (m_dataModel) {
        m_dataModel->validateCurrentKey();
    }
    updateUI();
}

#endif // WITH_XC_SSHAGENT