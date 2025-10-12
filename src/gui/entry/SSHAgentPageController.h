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

#ifndef KEEPASSX_SSHAGENTPAGECONTROLLER_H
#define KEEPASSX_SSHAGENTPAGECONTROLLER_H

#include "EntryPageController.h"
#include "config-keepassx.h"

#ifdef WITH_XC_SSHAGENT

#include <QPointer>
#include <QWidget>

class SSHAgentKeyDataModel;
class SSHAgentPageWidget;
class Entry;
class Database;

/**
 * Page controller for SSH Agent functionality.
 * Manages SSH key data and agent interaction separately from UI concerns.
 */
class SSHAgentPageController : public EntryPageController
{
    Q_OBJECT

public:
    explicit SSHAgentPageController(QObject* parent = nullptr);
    ~SSHAgentPageController() override;

    // EntryPageController interface
    void loadEntry(Entry* entry) override;
    bool saveEntry(Entry* entry) override;
    bool validateInput() override;
    void clear() override;
    QWidget* widget() override;
    QString displayName() const override;
    bool isEnabled(Database* database) const override;

private slots:
    void onAddKeyToAgent();
    void onRemoveKeyFromAgent();
    void onGenerateKey();
    void onCopyPublicKey();
    void onKeyDataChanged();

private:
    void updateUI();
    void connectSignals();
    void updateAgentStatus();

    QPointer<SSHAgentPageWidget> m_widget;
    QPointer<SSHAgentKeyDataModel> m_dataModel;
};

#endif // WITH_XC_SSHAGENT

#endif // KEEPASSX_SSHAGENTPAGECONTROLLER_H