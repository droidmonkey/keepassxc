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

#include "SearchManager.h"
#include "DatabaseWidget.h"

#include <QAction>

#include "core/EntrySearcher.h"
#include "core/Group.h"
#include "gui/entry/EntryView.h"

SearchManager::SearchManager(DatabaseWidget* parent)
    : QObject(parent)
    , m_databaseWidget(parent)
    , m_entrySearcher(new EntrySearcher(false))
    , m_searchLimitGroup(false)
    , m_searchActive(false)
{
    Q_ASSERT(m_databaseWidget);
}

SearchManager::~SearchManager() = default;

void SearchManager::search(const QString& searchText)
{
    if (searchText.isEmpty()) {
        endSearch();
        return;
    }

    if (!m_searchActive) {
        emit searchModeAboutToActivate();
        m_searchActive = true;
        emit searchModeActivated();
    }

    executeSearch(searchText);
}

void SearchManager::saveSearch(const QString& searchText)
{
    if (searchText.isEmpty()) {
        return;
    }

    // Implementation for saving search queries
    // This would typically involve storing in application settings or database
    // For now, this is a placeholder
    
    Q_UNUSED(searchText)
}

void SearchManager::deleteSearch(const QString& name)
{
    if (name.isEmpty()) {
        return;
    }

    // Implementation for deleting saved search queries
    // This would typically involve removing from application settings or database
    // For now, this is a placeholder
    
    Q_UNUSED(name)
}

void SearchManager::endSearch()
{
    if (!m_searchActive) {
        return;
    }

    emit listModeAboutToActivate();
    m_searchActive = false;
    m_lastSearchText.clear();
    
    // Reset entry view to show all entries
    auto entryView = m_databaseWidget->entryView();
    if (entryView && m_databaseWidget->currentGroup()) {
        // Display the current group to return from search mode
        entryView->displayGroup(m_databaseWidget->currentGroup());
    }
    
    emit listModeActivated();
    emit clearSearch();
}

void SearchManager::refreshSearch()
{
    if (!m_searchActive || m_lastSearchText.isEmpty()) {
        return;
    }

    executeSearch(m_lastSearchText);
}

void SearchManager::setSearchCaseSensitive(bool caseSensitive)
{
    if (m_entrySearcher) {
        m_entrySearcher->setCaseSensitive(caseSensitive);
        
        // Refresh current search if active
        if (m_searchActive) {
            refreshSearch();
        }
    }
}

void SearchManager::setSearchLimitGroup(bool limitToGroup)
{
    m_searchLimitGroup = limitToGroup;
    
    // Refresh current search if active
    if (m_searchActive) {
        refreshSearch();
    }
}

void SearchManager::filterByTag()
{
    // Implementation for tag-based filtering
    // This would involve showing only entries with specific tags
    // For now, this is a placeholder
}

void SearchManager::setTag(QAction* action)
{
    if (!action) {
        return;
    }

    QString tagName = action->data().toString();
    if (tagName.isEmpty()) {
        return;
    }

    // Implementation for setting tag filter
    // This would filter entries based on the selected tag
    // For now, this is a placeholder
    
    Q_UNUSED(tagName)
}

bool SearchManager::isSearchActive() const
{
    return m_searchActive;
}

QString SearchManager::getCurrentSearch() const
{
    return m_lastSearchText;
}

void SearchManager::setSearchStringForAutoType(const QString& search)
{
    m_searchStringForAutoType = search;
}

void SearchManager::onSearchTextChanged(const QString& text)
{
    search(text);
}

void SearchManager::executeSearch(const QString& searchText)
{
    if (!m_databaseWidget->database()) {
        return;
    }

    m_lastSearchText = searchText;

    // Determine search scope
    Group* searchGroup = m_databaseWidget->database()->rootGroup();
    if (m_searchLimitGroup && m_databaseWidget->currentGroup()) {
        searchGroup = m_databaseWidget->currentGroup();
    }

    // Perform the search
    QList<Entry*> searchResults;
    if (m_entrySearcher) {
        searchResults = m_entrySearcher->search(searchText, searchGroup);
    }

    // Update entry view with search results
    auto entryView = m_databaseWidget->entryView();
    if (entryView) {
        entryView->displaySearch(searchResults);
    }

    updateSearchState();
    emit searchResultsChanged(searchResults.size());
}

void SearchManager::updateSearchState()
{
    // Update any search-related UI elements
    // This could involve updating search labels, counters, etc.
    // For now, this is a placeholder
}

