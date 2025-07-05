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

#ifndef KEEPASSX_MAINWINDOWMENUMANAGER_H
#define KEEPASSX_MAINWINDOWMENUMANAGER_H

#include <QActionGroup>
#include <QMenu>
#include <QObject>
#include <QPointer>

namespace Ui
{
    class MainWindow;
}

class DatabaseTabWidget;

/**
 * MainWindowMenuManager handles menu-related functionality for the MainWindow.
 *
 * This class is responsible for:
 * - Creating and maintaining context menus (entry, group)
 * - Managing dynamic menu content (recent databases, copy attributes, tags)
 * - Setting up menu action groups
 * - Handling menu updates based on application state
 *
 * The class was extracted from MainWindow to improve code organization and
 * maintainability by following the Single Responsibility Principle.
 */
class MainWindowMenuManager : public QObject
{
    Q_OBJECT

public:
    explicit MainWindowMenuManager(Ui::MainWindow* ui, DatabaseTabWidget* tabWidget, QObject* parent = nullptr);
    ~MainWindowMenuManager() override;

    /**
     * Initialize all menus and menu action groups.
     * Must be called after the UI is set up.
     */
    void initializeMenus();

    /**
     * Update the recent databases menu with current list.
     */
    void updateLastDatabasesMenu();

    /**
     * Update copy attributes menu based on current entry selection.
     */
    void updateCopyAttributesMenu();

    /**
     * Update set tags menu based on current database tags and entry selection.
     */
    void updateSetTagsMenu();

    /**
     * Get the entry context menu.
     * @return Pointer to the entry context menu
     */
    QMenu* entryContextMenu() const
    {
        return m_entryContextMenu;
    }

    /**
     * Get the entry new context menu.
     * @return Pointer to the entry new context menu
     */
    QMenu* entryNewContextMenu() const
    {
        return m_entryNewContextMenu;
    }

    /**
     * Get the last databases action group.
     * @return Pointer to the last databases action group
     */
    QActionGroup* lastDatabasesActions() const
    {
        return m_lastDatabasesActions;
    }

    /**
     * Get the copy additional attributes action group.
     * @return Pointer to the copy additional attributes action group
     */
    QActionGroup* copyAdditionalAttributeActions() const
    {
        return m_copyAdditionalAttributeActions;
    }

    /**
     * Get the set tags menu action group.
     * @return Pointer to the set tags menu action group
     */
    QActionGroup* setTagsMenuActions() const
    {
        return m_setTagsMenuActions;
    }

signals:
    /**
     * Emitted when a recent database should be opened.
     * @param filePath The database file path to open
     */
    void openRecentDatabase(const QString& filePath);

    /**
     * Emitted when recent databases history should be cleared.
     */
    void clearLastDatabases();

private slots:
    void onOpenRecentDatabase(QAction* action);
    void onClearLastDatabases();

private:
    void createEntryContextMenu();
    void createEntryNewContextMenu();
    void setupMenuActionGroups();

    Ui::MainWindow* m_ui;
    DatabaseTabWidget* m_tabWidget;

    QPointer<QMenu> m_entryContextMenu;
    QPointer<QMenu> m_entryNewContextMenu;
    QPointer<QActionGroup> m_lastDatabasesActions;
    QPointer<QActionGroup> m_copyAdditionalAttributeActions;
    QPointer<QActionGroup> m_setTagsMenuActions;
    QPointer<QAction> m_clearHistoryAction;

    int m_countDefaultAttributes;
};

#endif // KEEPASSX_MAINWINDOWMENUMANAGER_H