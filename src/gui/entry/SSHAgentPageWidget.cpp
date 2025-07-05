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

#include "SSHAgentPageWidget.h"

#ifdef WITH_XC_SSHAGENT

#include "ui_EditEntryWidgetSSHAgent.h"
#include "SSHAgentKeyDataModel.h"

SSHAgentPageWidget::SSHAgentPageWidget(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::EditEntryWidgetSSHAgent)
{
    m_ui->setupUi(this);
    connectSignals();
}

SSHAgentPageWidget::~SSHAgentPageWidget()
{
    delete m_ui;
}

void SSHAgentPageWidget::setDataModel(SSHAgentKeyDataModel* model)
{
    if (m_dataModel) {
        m_dataModel->disconnect(this);
    }

    m_dataModel = model;

    if (m_dataModel) {
        connect(m_dataModel, &SSHAgentKeyDataModel::dataChanged,
                this, &SSHAgentPageWidget::onModelDataChanged);
        connect(m_dataModel, &SSHAgentKeyDataModel::keyDataChanged,
                this, &SSHAgentPageWidget::onModelKeyDataChanged);
        connect(m_dataModel, &SSHAgentKeyDataModel::agentStatusChanged,
                this, &SSHAgentPageWidget::onModelAgentStatusChanged);

        updateFromModel();
    }
}

void SSHAgentPageWidget::updateFromModel()
{
    if (!m_dataModel) {
        return;
    }

    // Update settings controls
    m_ui->enabledCheckBox->setChecked(m_dataModel->isEnabled());
    m_ui->addAtDatabaseOpenCheckBox->setChecked(m_dataModel->addAtDatabaseOpen());
    m_ui->removeAtDatabaseCloseCheckBox->setChecked(m_dataModel->removeAtDatabaseClose());
    m_ui->requireUserConfirmationCheckBox->setChecked(m_dataModel->requireUserConfirmation());
    m_ui->useLifetimeConstraintCheckBox->setChecked(m_dataModel->useLifetimeConstraint());
    m_ui->lifetimeConstraintSpinBox->setValue(m_dataModel->lifetimeConstraintDuration());

    // Update UI state
    bool enabled = m_dataModel->isEnabled();
    m_ui->settingsGroupBox->setEnabled(enabled);
    m_ui->keyInfoGroupBox->setEnabled(enabled);
    m_ui->actionsGroupBox->setEnabled(enabled);

    updateKeyInfoDisplay();
    updateButtonStates();
}

void SSHAgentPageWidget::connectSignals()
{
    // Settings connections
    connect(m_ui->enabledCheckBox, &QCheckBox::toggled,
            this, &SSHAgentPageWidget::onEnabledChanged);
    connect(m_ui->addAtDatabaseOpenCheckBox, &QCheckBox::toggled,
            this, &SSHAgentPageWidget::onSettingsChanged);
    connect(m_ui->removeAtDatabaseCloseCheckBox, &QCheckBox::toggled,
            this, &SSHAgentPageWidget::onSettingsChanged);
    connect(m_ui->requireUserConfirmationCheckBox, &QCheckBox::toggled,
            this, &SSHAgentPageWidget::onSettingsChanged);
    connect(m_ui->useLifetimeConstraintCheckBox, &QCheckBox::toggled,
            this, &SSHAgentPageWidget::onSettingsChanged);
    connect(m_ui->lifetimeConstraintSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SSHAgentPageWidget::onSettingsChanged);

    // Action button connections
    connect(m_ui->addToAgentButton, &QPushButton::clicked,
            this, &SSHAgentPageWidget::addKeyToAgentRequested);
    connect(m_ui->removeFromAgentButton, &QPushButton::clicked,
            this, &SSHAgentPageWidget::removeKeyFromAgentRequested);
    connect(m_ui->generateButton, &QPushButton::clicked,
            this, &SSHAgentPageWidget::generateKeyRequested);
    connect(m_ui->copyToClipboardButton, &QPushButton::clicked,
            this, &SSHAgentPageWidget::copyPublicKeyRequested);
}

void SSHAgentPageWidget::onEnabledChanged()
{
    if (m_dataModel) {
        m_dataModel->setEnabled(m_ui->enabledCheckBox->isChecked());
    }
}

void SSHAgentPageWidget::onSettingsChanged()
{
    if (!m_dataModel) {
        return;
    }

    m_dataModel->setAddAtDatabaseOpen(m_ui->addAtDatabaseOpenCheckBox->isChecked());
    m_dataModel->setRemoveAtDatabaseClose(m_ui->removeAtDatabaseCloseCheckBox->isChecked());
    m_dataModel->setRequireUserConfirmation(m_ui->requireUserConfirmationCheckBox->isChecked());
    m_dataModel->setUseLifetimeConstraint(m_ui->useLifetimeConstraintCheckBox->isChecked());
    m_dataModel->setLifetimeConstraintDuration(m_ui->lifetimeConstraintSpinBox->value());
}

void SSHAgentPageWidget::onModelDataChanged()
{
    updateFromModel();
}

void SSHAgentPageWidget::onModelKeyDataChanged()
{
    updateKeyInfoDisplay();
    updateButtonStates();
}

void SSHAgentPageWidget::onModelAgentStatusChanged()
{
    updateButtonStates();
}

void SSHAgentPageWidget::updateKeyInfoDisplay()
{
    if (!m_dataModel) {
        return;
    }

    if (m_dataModel->hasValidKey()) {
        m_ui->keyTypeTextLabel->setText(m_dataModel->keyType());
        m_ui->fingerprintTextLabel->setText(m_dataModel->fingerprint());
        m_ui->publicKeyTextEdit->setPlainText(m_dataModel->publicKey());
        m_ui->commentEdit->setText(m_dataModel->comment());
    } else {
        m_ui->keyTypeTextLabel->setText(tr("No key loaded"));
        m_ui->fingerprintTextLabel->clear();
        m_ui->publicKeyTextEdit->clear();
        m_ui->commentEdit->clear();
    }
}

void SSHAgentPageWidget::updateButtonStates()
{
    if (!m_dataModel) {
        return;
    }

    bool hasValidKey = m_dataModel->hasValidKey();
    bool isInAgent = m_dataModel->isKeyLoadedInAgent();

    m_ui->addToAgentButton->setEnabled(hasValidKey && !isInAgent);
    m_ui->removeFromAgentButton->setEnabled(hasValidKey && isInAgent);
    m_ui->copyToClipboardButton->setEnabled(hasValidKey);
}

#endif // WITH_XC_SSHAGENT