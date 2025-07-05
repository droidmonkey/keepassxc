/*
 * Copyright (C) 2024 KeePassXC Team <team@keepassxc.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 or (at your option)
 * version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ViewStateManager.h"
#include "DatabaseWidget.h"

#include "core/Entry.h"
#include "core/Group.h"
#include "gui/DatabaseOpenWidget.h"
#include "gui/dbsettings/DatabaseSettingsDialog.h"
#include "gui/entry/EditEntryWidget.h"
#include "gui/group/EditGroupWidget.h"
#include "gui/reports/ReportsDialog.h"

#ifdef WITH_XC_BROWSER_PASSKEYS
#include "gui/passkeys/PasskeyImporter.h"
#endif

ViewStateManager::ViewStateManager(DatabaseWidget* parent)
    : QObject(parent)
    , m_databaseWidget(parent)
    , m_currentMode(DatabaseWidget::Mode::None)
{
    Q_ASSERT(m_databaseWidget);
}

ViewStateManager::~ViewStateManager() = default;

DatabaseWidget::Mode ViewStateManager::currentMode() const
{
    return m_currentMode;
}

void ViewStateManager::switchToMainView(bool previousDialogAccepted)
{
    setCurrentMode(DatabaseWidget::Mode::ViewMode);
    activateChildWidget(m_databaseWidget->m_mainWidget);

    if (previousDialogAccepted) {
        // Handle any post-dialog actions
        m_databaseWidget->focusOnEntries();
    }

    emit currentModeChanged(m_currentMode);
}

void ViewStateManager::switchToEntryEdit()
{
    auto currentEntry = m_databaseWidget->currentSelectedEntry();
    if (currentEntry) {
        switchToEntryEdit(currentEntry, false);
    }
}

void ViewStateManager::switchToEntryEdit(Entry* entry, bool create)
{
    if (!entry) {
        return;
    }

    setCurrentMode(DatabaseWidget::Mode::EditEntryMode);

    auto editWidget = m_databaseWidget->m_editEntryWidget;
    if (editWidget) {
        editWidget->loadEntry(entry, create, false, QString(), m_databaseWidget->database());
        activateChildWidget(editWidget);
    }

    emit currentModeChanged(m_currentMode);
}

void ViewStateManager::switchToGroupEdit()
{
    auto currentGroup = m_databaseWidget->currentGroup();
    if (currentGroup) {
        switchToGroupEdit(currentGroup, false);
    }
}

void ViewStateManager::switchToGroupEdit(Group* group, bool create)
{
    if (!group) {
        return;
    }

    setCurrentMode(DatabaseWidget::Mode::EditGroupMode);

    auto editWidget = m_databaseWidget->m_editGroupWidget;
    if (editWidget) {
        editWidget->loadGroup(group, create, m_databaseWidget->database());
        activateChildWidget(editWidget);
    }

    emit currentModeChanged(m_currentMode);
}

void ViewStateManager::switchToDatabaseSettings()
{
    setCurrentMode(DatabaseWidget::Mode::DatabaseSettingsMode);

    auto settingsDialog = m_databaseWidget->m_databaseSettingDialog;
    if (settingsDialog) {
        settingsDialog->load(m_databaseWidget->database());
        activateChildWidget(settingsDialog);
    }

    emit currentModeChanged(m_currentMode);
}

void ViewStateManager::switchToDatabaseSecurity()
{
    // For now, just switch to database settings
    // The specific security page selection can be implemented later
    switchToDatabaseSettings();
}

void ViewStateManager::switchToDatabaseReports()
{
    setCurrentMode(DatabaseWidget::Mode::ReportsMode);

    auto reportsDialog = m_databaseWidget->m_reportsDialog;
    if (reportsDialog) {
        reportsDialog->load(m_databaseWidget->database());
        activateChildWidget(reportsDialog);
    }

    emit currentModeChanged(m_currentMode);
}

void ViewStateManager::switchToRemoteSettings()
{
    // This would typically open a remote settings dialog
    // For now, delegate to database settings
    switchToDatabaseSettings();
}

#ifdef WITH_XC_BROWSER_PASSKEYS
void ViewStateManager::switchToPasskeys()
{
    // Implementation for passkeys management mode
    // This would involve showing passkey management interface
    // For now, this is a placeholder
    setCurrentMode(DatabaseWidget::Mode::ViewMode);
    emit currentModeChanged(m_currentMode);
}
#endif

void ViewStateManager::switchToOpenDatabase()
{
    setCurrentMode(DatabaseWidget::Mode::LockedMode);

    auto openWidget = m_databaseWidget->m_databaseOpenWidget;
    if (openWidget) {
        openWidget->load(m_databaseWidget->database()->filePath());
        activateChildWidget(openWidget);
    }

    emit currentModeChanged(m_currentMode);
}

void ViewStateManager::switchToOpenDatabase(const QString& filePath)
{
    setCurrentMode(DatabaseWidget::Mode::LockedMode);

    auto openWidget = m_databaseWidget->m_databaseOpenWidget;
    if (openWidget) {
        openWidget->load(filePath);
        activateChildWidget(openWidget);
    }

    emit currentModeChanged(m_currentMode);
}

void ViewStateManager::switchToOpenDatabase(const QString& filePath, const QString& password, const QString& keyFile)
{
    Q_UNUSED(password) // These parameters may be used in future implementation
    Q_UNUSED(keyFile)

    // For now, just switch to open database with the file path
    switchToOpenDatabase(filePath);
}

void ViewStateManager::switchToHistoryView(Entry* entry)
{
    if (!entry) {
        return;
    }

    setCurrentMode(DatabaseWidget::Mode::EditEntryMode);

    auto historyWidget = m_databaseWidget->m_historyEditEntryWidget;
    if (historyWidget) {
        historyWidget->loadEntry(entry, false, true, QString(), m_databaseWidget->database());
        activateChildWidget(historyWidget);
    }

    emit currentModeChanged(m_currentMode);
}

void ViewStateManager::clearAllWidgets()
{
    // Clear state from all edit widgets
    auto editEntryWidget = m_databaseWidget->m_editEntryWidget;
    if (editEntryWidget) {
        editEntryWidget->clear();
    }

    auto editGroupWidget = m_databaseWidget->m_editGroupWidget;
    if (editGroupWidget) {
        editGroupWidget->clear();
    }

    auto historyWidget = m_databaseWidget->m_historyEditEntryWidget;
    if (historyWidget) {
        historyWidget->clear();
    }

    // Switch back to main view
    switchToMainView();
}

bool ViewStateManager::isEditWidgetModified() const
{
    auto editEntryWidget = m_databaseWidget->m_editEntryWidget;
    if (editEntryWidget && editEntryWidget->isVisible()) {
        // Check if the widget has been modified
        // This may need to be implemented based on the actual EditEntryWidget API
        return false; // Placeholder
    }

    auto editGroupWidget = m_databaseWidget->m_editGroupWidget;
    if (editGroupWidget && editGroupWidget->isVisible()) {
        // Check if the widget has been modified
        // This may need to be implemented based on the actual EditGroupWidget API
        return false; // Placeholder
    }

    return false;
}

bool ViewStateManager::isEntryEditActive() const
{
    return m_currentMode == DatabaseWidget::Mode::EditEntryMode;
}

bool ViewStateManager::isGroupEditActive() const
{
    return m_currentMode == DatabaseWidget::Mode::EditGroupMode;
}

void ViewStateManager::emitCurrentModeChanged()
{
    emit currentModeChanged(m_currentMode);
}

void ViewStateManager::setCurrentMode(DatabaseWidget::Mode mode)
{
    if (m_currentMode != mode) {
        m_currentMode = mode;
        emitCurrentModeChanged();
    }
}

int ViewStateManager::addChildWidget(QWidget* widget)
{
    if (!widget) {
        return -1;
    }

    return m_databaseWidget->addWidget(widget);
}

void ViewStateManager::activateChildWidget(QWidget* widget)
{
    if (!widget) {
        return;
    }

    m_databaseWidget->setCurrentWidget(widget);
    widget->setFocus();
}
