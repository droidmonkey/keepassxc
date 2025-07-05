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

#ifndef KEEPASSX_SEARCHMANAGER_H
#define KEEPASSX_SEARCHMANAGER_H

#include <QObject>
#include <QString>
#include <QScopedPointer>

class DatabaseWidget;
class EntrySearcher;
class QAction;

/**
 * @brief Manages search functionality for DatabaseWidget
 * 
 * This class extracts search-related operations from the DatabaseWidget
 * to improve separation of concerns and maintainability.
 * 
 * Responsibilities:
 * - Text-based entry searching
 * - Search state management (active/inactive)
 * - Search options (case sensitivity, group limiting)
 * - Tag-based filtering
 * - Search history and saved searches
 */
class SearchManager : public QObject
{
    Q_OBJECT

public:
    explicit SearchManager(DatabaseWidget* parent);
    ~SearchManager() override;

    /**
     * @brief Perform a search with the given text
     * @param searchText text to search for
     */
    void search(const QString& searchText);

    /**
     * @brief Save the current search with a name
     * @param searchText text to save
     */
    void saveSearch(const QString& searchText);

    /**
     * @brief Delete a saved search
     * @param name name of the search to delete
     */
    void deleteSearch(const QString& name);

    /**
     * @brief End the current search and return to normal view
     */
    void endSearch();

    /**
     * @brief Refresh/re-execute the current search
     */
    void refreshSearch();

    /**
     * @brief Set case sensitivity for searches
     * @param caseSensitive true to enable case-sensitive searching
     */
    void setSearchCaseSensitive(bool caseSensitive);

    /**
     * @brief Set whether search is limited to current group
     * @param limitToGroup true to limit search to current group
     */
    void setSearchLimitGroup(bool limitToGroup);

    /**
     * @brief Filter entries by tag
     */
    void filterByTag();

    /**
     * @brief Set tag filter from action
     * @param action action containing tag information
     */
    void setTag(QAction* action);

    /**
     * @brief Check if search is currently active
     * @return true if search is active
     */
    bool isSearchActive() const;

    /**
     * @brief Get the current search text
     * @return current search string
     */
    QString getCurrentSearch() const;

    /**
     * @brief Set search string for auto-type functionality
     * @param search search string for auto-type
     */
    void setSearchStringForAutoType(const QString& search);

signals:
    /**
     * @brief Emitted when search mode is about to be activated
     */
    void searchModeAboutToActivate();

    /**
     * @brief Emitted when search mode is activated
     */
    void searchModeActivated();

    /**
     * @brief Emitted when returning to list mode
     */
    void listModeAboutToActivate();

    /**
     * @brief Emitted when list mode is activated
     */
    void listModeActivated();

    /**
     * @brief Emitted when search should be cleared
     */
    void clearSearch();

    /**
     * @brief Emitted when search results change
     * @param resultCount number of search results
     */
    void searchResultsChanged(int resultCount);

private slots:
    /**
     * @brief Handle search text changes from UI
     * @param text new search text
     */
    void onSearchTextChanged(const QString& text);

private:
    /**
     * @brief Execute the search with current parameters
     * @param searchText text to search for
     */
    void executeSearch(const QString& searchText);

    /**
     * @brief Update search UI state
     */
    void updateSearchState();

    DatabaseWidget* m_databaseWidget;           ///< Reference to parent DatabaseWidget
    QScopedPointer<EntrySearcher> m_entrySearcher; ///< Search engine instance
    
    QString m_lastSearchText;                   ///< Last executed search text
    QString m_searchStringForAutoType;          ///< Search string for auto-type
    bool m_searchLimitGroup;                    ///< Whether search is limited to current group
    bool m_searchActive;                        ///< Whether search is currently active

    Q_DISABLE_COPY(SearchManager)
};

#endif // KEEPASSX_SEARCHMANAGER_H