/*
 *  Copyright (C) 2014 Florian Geyer <debfx@fobos.de>
 *  Copyright (C) 2017 KeePassXC Team <team@keepassxc.org>
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

#ifndef KEEPASSX_ENTRYSEARCHER_H
#define KEEPASSX_ENTRYSEARCHER_H

#include <QRegularExpression>

class Group;
class Entry;

class EntrySearcher
{
public:
    enum class Field
    {
        Undefined,
        Title,
        Username,
        Password,
        Url,
        Notes,
        AttributeKV,
        Attachment,
        AttributeValue,
        Group,
        Tag,
        Is,
        Has,
        Uuid
    };

    struct SearchTerm
    {
        Field field;
        // only used when field == Field::AttributeValue
        QString word;
        QRegularExpression regex;
        bool exclude;
    };

    struct SearchResult
    {
        Entry* entry;
        double relevanceScore;

        SearchResult(Entry* e = nullptr, double score = 0.0)
            : entry(e)
            , relevanceScore(score)
        {
        }

        // For sorting by relevance (highest first), then by modification time (newest first)
        bool operator<(const SearchResult& other) const;
    };

    explicit EntrySearcher(bool caseSensitive = false, bool skipProtected = false);

    QList<Entry*> search(const QList<SearchTerm>& searchTerms, const Group* baseGroup, bool forceSearch = false);
    QList<Entry*> search(const QString& searchString, const Group* baseGroup, bool forceSearch = false);
    QList<Entry*> repeat(const Group* baseGroup, bool forceSearch = false);

    QList<Entry*> searchEntries(const QList<SearchTerm>& searchTerms, const QList<Entry*>& entries);
    QList<Entry*> searchEntries(const QString& searchString, const QList<Entry*>& entries);
    QList<Entry*> repeatEntries(const QList<Entry*>& entries);

    // New methods that return relevance-scored results
    QList<SearchResult>
    searchWithScore(const QList<SearchTerm>& searchTerms, const Group* baseGroup, bool forceSearch = false);
    QList<SearchResult> searchWithScore(const QString& searchString, const Group* baseGroup, bool forceSearch = false);
    QList<SearchResult> repeatWithScore(const Group* baseGroup, bool forceSearch = false);

    QList<SearchResult> searchEntriesWithScore(const QList<SearchTerm>& searchTerms, const QList<Entry*>& entries);
    QList<SearchResult> searchEntriesWithScore(const QString& searchString, const QList<Entry*>& entries);
    QList<SearchResult> repeatEntriesWithScore(const QList<Entry*>& entries);

    void setCaseSensitive(bool state);
    bool isCaseSensitive() const;

private:
    bool searchEntryImpl(const Entry* entry);
    double searchEntryWithScore(const Entry* entry);
    double calculateFieldScore(Field field, const QString& fieldValue, const SearchTerm& term);
    double getFieldPriority(Field field);
    double getMatchQuality(const QString& fieldValue, const QRegularExpression& regex);
    void parseSearchTerms(const QString& searchString);

    bool m_caseSensitive;
    bool m_skipProtected;
    QList<SearchTerm> m_searchTerms;

    friend class TestEntrySearcher;
};

#endif // KEEPASSX_ENTRYSEARCHER_H
