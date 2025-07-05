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

#ifndef KEEPASSX_MAINWINDOWTOOLBARMANAGER_H
#define KEEPASSX_MAINWINDOWTOOLBARMANAGER_H

#include <QObject>
#include <QPointer>

class QAction;
class QMenu;
class QToolButton;
class SearchWidget;

namespace Ui
{
    class MainWindow;
}

class SignalMultiplexer;

/**
 * MainWindowToolbarManager handles toolbar-related functionality for the MainWindow.
 *
 * This class is responsible for:
 * - Setting up toolbar widgets and buttons
 * - Managing toolbar visibility and state
 * - Handling toolbar separator visibility
 * - Configuring toolbar properties (movable, style, etc.)
 * - Integrating search widget into toolbar
 *
 * The class was extracted from MainWindow to improve code organization and
 * maintainability by following the Single Responsibility Principle.
 */
class MainWindowToolbarManager : public QObject
{
    Q_OBJECT

public:
    explicit MainWindowToolbarManager(Ui::MainWindow* ui, SignalMultiplexer* multiplexer, QObject* parent = nullptr);
    ~MainWindowToolbarManager() override;

    /**
     * Initialize toolbar setup and configuration.
     * Must be called after the UI is set up.
     */
    void initializeToolbar();

    /**
     * Setup the search widget in the toolbar.
     * @param searchWidget The search widget to add to toolbar
     * @return The action that represents the search widget in the toolbar
     */
    QAction* setupSearchWidget(SearchWidget* searchWidget);

    /**
     * Setup toolbar button menus (auto-type, database lock, etc.).
     */
    void setupToolbarMenus();

    /**
     * Update toolbar separator visibility based on application state.
     */
    void updateToolbarSeparatorVisibility();

    /**
     * Apply toolbar-related settings changes.
     */
    void applyToolbarSettings();

    /**
     * Setup toolbar visibility toggle in View menu.
     */
    void setupToolbarVisibilityToggle();

private slots:
    void onSearchWidgetCanceled();
    void onSearchWidgetLostFocus();
    void onToolbarVisibilityToggled(bool checked);

private:
    void setupAutoTypeMenu();
    void setupDatabaseLockMenu();
    void updateToolbarVisibility();

    Ui::MainWindow* m_ui;
    SignalMultiplexer* m_actionMultiplexer;

    QPointer<QAction> m_searchWidgetAction;
    bool m_showToolbarSeparator;
};

#endif // KEEPASSX_MAINWINDOWTOOLBARMANAGER_H