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

#ifndef KEEPASSX_SSHAGENTPAGEWIDGET_H
#define KEEPASSX_SSHAGENTPAGEWIDGET_H

#include "config-keepassx.h"

#ifdef WITH_XC_SSHAGENT

#include <QWidget>
#include <QPointer>

class SSHAgentKeyDataModel;

namespace Ui {
class EditEntryWidgetSSHAgent;
}

/**
 * UI widget for SSH Agent page.
 * Handles only presentation logic, delegates business logic to the data model.
 */
class SSHAgentPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SSHAgentPageWidget(QWidget* parent = nullptr);
    ~SSHAgentPageWidget() override;

    /**
     * Set the data model for this widget
     */
    void setDataModel(SSHAgentKeyDataModel* model);

    /**
     * Update the UI from the current data model
     */
    void updateFromModel();

signals:
    void addKeyToAgentRequested();
    void removeKeyFromAgentRequested();
    void generateKeyRequested();
    void copyPublicKeyRequested();

private slots:
    void onEnabledChanged();
    void onSettingsChanged();
    void onModelDataChanged();
    void onModelKeyDataChanged();
    void onModelAgentStatusChanged();

private:
    void connectSignals();
    void updateKeyInfoDisplay();
    void updateButtonStates();

    Ui::EditEntryWidgetSSHAgent* m_ui;
    QPointer<SSHAgentKeyDataModel> m_dataModel;
};

#endif // WITH_XC_SSHAGENT

#endif // KEEPASSX_SSHAGENTPAGEWIDGET_H