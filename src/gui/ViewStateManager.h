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

#ifndef KEEPASSX_VIEWSTATEMANAGER_H
#define KEEPASSX_VIEWSTATEMANAGER_H

#include "DatabaseWidget.h"
#include <QObject>

class Entry;
class Group;

/**
 * @brief Manages view state transitions for DatabaseWidget
 *
 * This class extracts view state management and mode transitions
 * from the DatabaseWidget to improve separation of concerns and maintainability.
 *
 * Responsibilities:
 * - Managing different widget modes (View, Edit, Locked, Reports, Settings)
 * - Coordinating view transitions
 * - Maintaining view state consistency
 * - Handling widget stacking and visibility
 */
class ViewStateManager : public QObject
{
    Q_OBJECT

public:
    explicit ViewStateManager(DatabaseWidget* parent);
    ~ViewStateManager() override;

    /**
     * @brief Get the current widget mode
     * @return current mode
     */
    DatabaseWidget::Mode currentMode() const;

    /**
     * @brief Switch to main view mode
     * @param previousDialogAccepted whether previous dialog was accepted
     */
    void switchToMainView(bool previousDialogAccepted = false);

    /**
     * @brief Switch to entry edit mode
     */
    void switchToEntryEdit();

    /**
     * @brief Switch to entry edit mode for specific entry
     * @param entry entry to edit
     * @param create whether this is a new entry being created
     */
    void switchToEntryEdit(Entry* entry, bool create = false);

    /**
     * @brief Switch to group edit mode
     */
    void switchToGroupEdit();

    /**
     * @brief Switch to group edit mode for specific group
     * @param group group to edit
     * @param create whether this is a new group being created
     */
    void switchToGroupEdit(Group* group, bool create = false);

    /**
     * @brief Switch to database settings mode
     */
    void switchToDatabaseSettings();

    /**
     * @brief Switch to database security settings
     */
    void switchToDatabaseSecurity();

    /**
     * @brief Switch to database reports mode
     */
    void switchToDatabaseReports();

    /**
     * @brief Switch to remote settings mode
     */
    void switchToRemoteSettings();

#ifdef WITH_XC_BROWSER_PASSKEYS
    /**
     * @brief Switch to passkeys management mode
     */
    void switchToPasskeys();
#endif

    /**
     * @brief Switch to database open/unlock mode
     */
    void switchToOpenDatabase();

    /**
     * @brief Switch to database open mode with specific file
     * @param filePath path to database file
     */
    void switchToOpenDatabase(const QString& filePath);

    /**
     * @brief Switch to database open mode with credentials
     * @param filePath path to database file
     * @param password database password
     * @param keyFile path to key file
     */
    void switchToOpenDatabase(const QString& filePath, const QString& password, const QString& keyFile);

    /**
     * @brief Switch to entry history view
     * @param entry entry to show history for
     */
    void switchToHistoryView(Entry* entry);

    /**
     * @brief Clear all widgets and reset to default state
     */
    void clearAllWidgets();

    /**
     * @brief Check if entry edit widget is currently modified
     * @return true if edit widget has unsaved changes
     */
    bool isEditWidgetModified() const;

    /**
     * @brief Check if currently in entry edit mode
     * @return true if in entry edit mode
     */
    bool isEntryEditActive() const;

    /**
     * @brief Check if currently in group edit mode
     * @return true if in group edit mode
     */
    bool isGroupEditActive() const;

signals:
    /**
     * @brief Emitted when the current mode changes
     * @param mode new mode
     */
    void currentModeChanged(DatabaseWidget::Mode mode);

    /**
     * @brief Emitted when switching back to entry edit from history
     */
    void switchBackToEntryEdit();

private slots:
    /**
     * @brief Handle entry widget changes and emit mode change signal
     */
    void emitCurrentModeChanged();

private:
    /**
     * @brief Set the current widget mode
     * @param mode new mode to set
     */
    void setCurrentMode(DatabaseWidget::Mode mode);

    /**
     * @brief Add a child widget to the stacked widget
     * @param widget widget to add
     * @return index of added widget
     */
    int addChildWidget(QWidget* widget);

    /**
     * @brief Activate a specific child widget
     * @param widget widget to activate
     */
    void activateChildWidget(QWidget* widget);

    DatabaseWidget* m_databaseWidget; ///< Reference to parent DatabaseWidget
    DatabaseWidget::Mode m_currentMode; ///< Current widget mode

    Q_DISABLE_COPY(ViewStateManager)
};

#endif // KEEPASSX_VIEWSTATEMANAGER_H