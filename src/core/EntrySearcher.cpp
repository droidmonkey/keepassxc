/*
 *  Copyright (C) 2014 Florian Geyer <blueice@fobos.de>
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

#include "EntrySearcher.h"

#include "PasswordHealth.h"
#include "core/Entry.h"
#include "core/Group.h"
#include "core/Tools.h"
#include <algorithm>

bool EntrySearcher::SearchResult::operator<(const SearchResult& other) const
{
    if (qFuzzyCompare(relevanceScore, other.relevanceScore)) {
        // If scores are equal, sort by modification time (newest first)
        return entry && other.entry && entry->timeInfo().lastModificationTime() < other.entry->timeInfo().lastModificationTime();
    }
    return relevanceScore < other.relevanceScore;
}

EntrySearcher::EntrySearcher(bool caseSensitive, bool skipProtected)
    : m_caseSensitive(caseSensitive)
    , m_skipProtected(skipProtected)
{
}

/**
 * Search group, and its children, directly by provided search terms
 * @param searchTerms  search terms
 * @param baseGroup group to start search from, cannot be null
 * @param forceSearch ignore group search settings
 * @return list of entries that match the search terms
 */
QList<Entry*> EntrySearcher::search(const QList<SearchTerm>& searchTerms, const Group* baseGroup, bool forceSearch)
{
    Q_ASSERT(baseGroup);
    m_searchTerms = searchTerms;
    return repeat(baseGroup, forceSearch);
}

/**
 * Search group, and its children, by parsing the provided search
 * string for search terms.
 *
 * @param searchString search terms
 * @param baseGroup group to start search from, cannot be null
 * @param forceSearch ignore group search settings
 * @return list of entries that match the search terms
 */
QList<Entry*> EntrySearcher::search(const QString& searchString, const Group* baseGroup, bool forceSearch)
{
    Q_ASSERT(baseGroup);
    parseSearchTerms(searchString);
    return repeat(baseGroup, forceSearch);
}

/**
 * Repeat the last search starting from the given group
 *
 * @param baseGroup group to start search from, cannot be null
 * @param forceSearch ignore group search settings
 * @return list of entries that match the search terms
 */
QList<Entry*> EntrySearcher::repeat(const Group* baseGroup, bool forceSearch)
{
    Q_ASSERT(baseGroup);

    QList<Entry*> results;
    for (const auto group : baseGroup->groupsRecursive(true)) {
        if (forceSearch || group->resolveSearchingEnabled()) {
            for (const auto entry : group->entries()) {
                if (searchEntryImpl(entry)) {
                    results.append(entry);
                }
            }
        }
    }
    return results;
}

/**
 * Search group and its children with relevance scoring
 *
 * @param searchTerms search terms
 * @param baseGroup group to start search from, cannot be null
 * @param forceSearch ignore group search settings
 * @return list of search results with relevance scores, sorted by score
 */
QList<EntrySearcher::SearchResult> EntrySearcher::searchWithScore(const QList<SearchTerm>& searchTerms, const Group* baseGroup, bool forceSearch)
{
    Q_ASSERT(baseGroup);
    m_searchTerms = searchTerms;
    return repeatWithScore(baseGroup, forceSearch);
}

/**
 * Search group and its children with relevance scoring by parsing search string
 *
 * @param searchString search terms
 * @param baseGroup group to start search from, cannot be null
 * @param forceSearch ignore group search settings
 * @return list of search results with relevance scores, sorted by score
 */
QList<EntrySearcher::SearchResult> EntrySearcher::searchWithScore(const QString& searchString, const Group* baseGroup, bool forceSearch)
{
    Q_ASSERT(baseGroup);
    parseSearchTerms(searchString);
    return repeatWithScore(baseGroup, forceSearch);
}

/**
 * Repeat the last search with relevance scoring
 *
 * @param baseGroup group to start search from, cannot be null
 * @param forceSearch ignore group search settings
 * @return list of search results with relevance scores, sorted by score
 */
QList<EntrySearcher::SearchResult> EntrySearcher::repeatWithScore(const Group* baseGroup, bool forceSearch)
{
    Q_ASSERT(baseGroup);

    QList<SearchResult> results;
    for (const auto group : baseGroup->groupsRecursive(true)) {
        if (forceSearch || group->resolveSearchingEnabled()) {
            for (const auto entry : group->entries()) {
                double score = searchEntryWithScore(entry);
                if (score > 0.0) {
                    results.append(SearchResult(entry, score));
                }
            }
        }
    }
    
    // Sort by relevance score (highest first), then by modification time (newest first)
    std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        return b < a;  // Reverse order for descending sort
    });
    
    return results;
}

/**
 * Search provided entries with relevance scoring
 *
 * @param searchTerms search terms
 * @param entries list of entries to include in the search
 * @return list of search results with relevance scores, sorted by score
 */
QList<EntrySearcher::SearchResult> EntrySearcher::searchEntriesWithScore(const QList<SearchTerm>& searchTerms, const QList<Entry*>& entries)
{
    m_searchTerms = searchTerms;
    return repeatEntriesWithScore(entries);
}

/**
 * Search provided entries with relevance scoring by parsing search string
 *
 * @param searchString search terms
 * @param entries list of entries to include in the search
 * @return list of search results with relevance scores, sorted by score
 */
QList<EntrySearcher::SearchResult> EntrySearcher::searchEntriesWithScore(const QString& searchString, const QList<Entry*>& entries)
{
    parseSearchTerms(searchString);
    return repeatEntriesWithScore(entries);
}

/**
 * Repeat the last search with relevance scoring on provided entries
 *
 * @param entries list of entries to include in the search
 * @return list of search results with relevance scores, sorted by score
 */
QList<EntrySearcher::SearchResult> EntrySearcher::repeatEntriesWithScore(const QList<Entry*>& entries)
{
    QList<SearchResult> results;
    for (auto* entry : entries) {
        double score = searchEntryWithScore(entry);
        if (score > 0.0) {
            results.append(SearchResult(entry, score));
        }
    }
    
    // Sort by relevance score (highest first), then by modification time (newest first)
    std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        return b < a;  // Reverse order for descending sort
    });
    
    return results;
}

/**
 * Search provided entries by the provided search terms
 *
 * @param searchTerms search terms
 * @param entries list of entries to include in the search
 * @return list of entries that match the search terms
 */
QList<Entry*> EntrySearcher::searchEntries(const QList<SearchTerm>& searchTerms, const QList<Entry*>& entries)
{
    m_searchTerms = searchTerms;
    return repeatEntries(entries);
}

/**
 * Search provided entries by parsing the search string
 * for search terms.
 *
 * @param searchString search terms
 * @param entries list of entries to include in the search
 * @return list of entries that match the search terms
 */
QList<Entry*> EntrySearcher::searchEntries(const QString& searchString, const QList<Entry*>& entries)
{
    parseSearchTerms(searchString);
    return repeatEntries(entries);
}

/**
 * Repeat the last search on the given entries
 *
 * @param entries list of entries to include in the search
 * @return list of entries that match the search terms
 */
QList<Entry*> EntrySearcher::repeatEntries(const QList<Entry*>& entries)
{
    QList<Entry*> results;
    for (auto* entry : entries) {
        if (searchEntryImpl(entry)) {
            results.append(entry);
        }
    }
    return results;
}

/**
 * Set the next search to be case sensitive or not
 *
 * @param state
 */
void EntrySearcher::setCaseSensitive(bool state)
{
    m_caseSensitive = state;
}

bool EntrySearcher::isCaseSensitive() const
{
    return m_caseSensitive;
}

bool EntrySearcher::searchEntryImpl(const Entry* entry)
{
    // Pre-load in case they are needed
    auto attributes_keys = entry->attributes()->customKeys();
    auto attributes = QStringList(attributes_keys + entry->attributes()->values(attributes_keys));
    auto attachments = QStringList(entry->attachments()->keys());
    // Build a group hierarchy to allow searching for e.g. /group1/subgroup*
    QString hierarchy;
    if (entry->group()) {
        hierarchy = entry->group()->hierarchy().join('/').prepend("/");
    }

    // By default, empty term matches every entry.
    // However when skipping protected fields, we will reject everything instead
    bool found = !m_skipProtected;
    for (const auto& term : m_searchTerms) {
        switch (term.field) {
        case Field::Title:
            found = term.regex.match(entry->resolvePlaceholder(entry->title())).hasMatch();
            break;
        case Field::Username:
            found = term.regex.match(entry->resolvePlaceholder(entry->username())).hasMatch();
            break;
        case Field::Password:
            if (m_skipProtected) {
                continue;
            }
            found = term.regex.match(entry->resolvePlaceholder(entry->password())).hasMatch();
            break;
        case Field::Url:
            found = term.regex.match(entry->resolvePlaceholder(entry->url())).hasMatch();
            break;
        case Field::Notes:
            found = term.regex.match(entry->notes()).hasMatch();
            break;
        case Field::AttributeKV:
            found = !attributes.filter(term.regex).empty();
            break;
        case Field::Attachment:
            found = !attachments.filter(term.regex).empty();
            break;
        case Field::AttributeValue:
            if (m_skipProtected && entry->attributes()->isProtected(term.word)) {
                continue;
            }
            found = entry->attributes()->contains(term.word)
                    && term.regex.match(entry->attributes()->value(term.word)).hasMatch();
            break;
        case Field::Group:
            // Match against the full hierarchy if the word contains a '/' otherwise just the group name
            if (term.word.contains('/')) {
                found = term.regex.match(hierarchy).hasMatch();
            } else if (entry->group()) {
                found = term.regex.match(entry->group()->name()).hasMatch();
            }
            break;
        case Field::Tag:
            found = entry->tagList().indexOf(term.regex) != -1;
            break;
        case Field::Is:
            if (term.word.startsWith("expired", Qt::CaseInsensitive)) {
                auto days = 0;
                auto parts = term.word.split("-", Qt::SkipEmptyParts);
                if (parts.length() >= 2) {
                    days = parts[1].toInt();
                }
                found = entry->willExpireInDays(days) && !entry->isRecycled();
                break;
            } else if (term.word.compare("weak", Qt::CaseInsensitive) == 0) {
                if (!entry->excludeFromReports() && !entry->password().isEmpty() && !entry->isExpired()) {
                    const auto quality = entry->passwordHealth()->quality();
                    if (quality == PasswordHealth::Quality::Bad || quality == PasswordHealth::Quality::Poor
                        || quality == PasswordHealth::Quality::Weak) {
                        found = true;
                        break;
                    }
                }
            }
            found = false;
            break;
        case Field::Has:
            if (term.word.compare("totp", Qt::CaseInsensitive) == 0) {
                found = entry->hasTotp();
                break;
            }
            found = false;
            break;
        case Field::Uuid:
            found = term.regex.match(entry->uuidToHex()).hasMatch();
            break;
        default:
            // Terms without a specific field try to match title, username, url, and notes
            found = term.regex.match(entry->resolvePlaceholder(entry->title())).hasMatch()
                    || term.regex.match(entry->resolvePlaceholder(entry->username())).hasMatch()
                    || term.regex.match(entry->resolvePlaceholder(entry->url())).hasMatch()
                    || entry->tagList().indexOf(term.regex) != -1 || term.regex.match(entry->notes()).hasMatch();
        }

        // negate the result if exclude:
        // * if found and not excluding, the entry matches
        // * if didn't found but excluding, the entry also matches
        found = (found && !term.exclude) || (!found && term.exclude);

        // short circuit if we failed the match
        if (!found) {
            return false;
        }
    }

    return found;
}

double EntrySearcher::searchEntryWithScore(const Entry* entry)
{
    // Pre-load in case they are needed
    auto attributes_keys = entry->attributes()->customKeys();
    auto attributes = QStringList(attributes_keys + entry->attributes()->values(attributes_keys));
    auto attachments = QStringList(entry->attachments()->keys());
    // Build a group hierarchy to allow searching for e.g. /group1/subgroup*
    QString hierarchy;
    if (entry->group()) {
        hierarchy = entry->group()->hierarchy().join('/').prepend("/");
    }

    // By default, empty term matches every entry with score 1.0
    // However when skipping protected fields, we will reject everything instead
    double totalScore = m_skipProtected ? 0.0 : 1.0;
    int matchedTerms = 0;
    
    for (const auto& term : m_searchTerms) {
        double termScore = 0.0;
        
        switch (term.field) {
        case Field::Title:
            termScore = calculateFieldScore(Field::Title, entry->resolvePlaceholder(entry->title()), term);
            break;
        case Field::Username:
            termScore = calculateFieldScore(Field::Username, entry->resolvePlaceholder(entry->username()), term);
            break;
        case Field::Password:
            if (m_skipProtected) {
                continue;
            }
            termScore = calculateFieldScore(Field::Password, entry->resolvePlaceholder(entry->password()), term);
            break;
        case Field::Url:
            termScore = calculateFieldScore(Field::Url, entry->resolvePlaceholder(entry->url()), term);
            break;
        case Field::Notes:
            termScore = calculateFieldScore(Field::Notes, entry->notes(), term);
            break;
        case Field::AttributeKV:
            {
                bool found = !attributes.filter(term.regex).empty();
                if (found) {
                    termScore = getFieldPriority(Field::AttributeKV) * getMatchQuality(attributes.join(" "), term.regex);
                }
            }
            break;
        case Field::Attachment:
            {
                bool found = !attachments.filter(term.regex).empty();
                if (found) {
                    termScore = getFieldPriority(Field::Attachment) * getMatchQuality(attachments.join(" "), term.regex);
                }
            }
            break;
        case Field::AttributeValue:
            if (m_skipProtected && entry->attributes()->isProtected(term.word)) {
                continue;
            }
            if (entry->attributes()->contains(term.word)) {
                termScore = calculateFieldScore(Field::AttributeValue, entry->attributes()->value(term.word), term);
            }
            break;
        case Field::Group:
            // Match against the full hierarchy if the word contains a '/' otherwise just the group name
            if (term.word.contains('/')) {
                termScore = calculateFieldScore(Field::Group, hierarchy, term);
            } else if (entry->group()) {
                termScore = calculateFieldScore(Field::Group, entry->group()->name(), term);
            }
            break;
        case Field::Tag:
            {
                int tagIndex = entry->tagList().indexOf(term.regex);
                if (tagIndex != -1) {
                    termScore = getFieldPriority(Field::Tag) * 10.0;  // Exact match for tags
                }
            }
            break;
        case Field::Is:
            if (term.word.startsWith("expired", Qt::CaseInsensitive)) {
                auto days = 0;
                auto parts = term.word.split("-", Qt::SkipEmptyParts);
                if (parts.length() >= 2) {
                    days = parts[1].toInt();
                }
                if (entry->willExpireInDays(days) && !entry->isRecycled()) {
                    termScore = getFieldPriority(Field::Is) * 10.0;
                }
                break;
            } else if (term.word.compare("weak", Qt::CaseInsensitive) == 0) {
                if (!entry->excludeFromReports() && !entry->password().isEmpty() && !entry->isExpired()) {
                    const auto quality = entry->passwordHealth()->quality();
                    if (quality == PasswordHealth::Quality::Bad || quality == PasswordHealth::Quality::Poor
                        || quality == PasswordHealth::Quality::Weak) {
                        termScore = getFieldPriority(Field::Is) * 10.0;
                        break;
                    }
                }
            }
            termScore = 0.0;
            break;
        case Field::Has:
            if (term.word.compare("totp", Qt::CaseInsensitive) == 0) {
                if (entry->hasTotp()) {
                    termScore = getFieldPriority(Field::Has) * 10.0;
                }
                break;
            }
            termScore = 0.0;
            break;
        case Field::Uuid:
            termScore = calculateFieldScore(Field::Uuid, entry->uuidToHex(), term);
            break;
        default:
            // Terms without a specific field try to match title, username, url, notes, and tags
            // Take the highest scoring field
            double titleScore = calculateFieldScore(Field::Title, entry->resolvePlaceholder(entry->title()), term);
            double usernameScore = calculateFieldScore(Field::Username, entry->resolvePlaceholder(entry->username()), term);
            double urlScore = calculateFieldScore(Field::Url, entry->resolvePlaceholder(entry->url()), term);
            double notesScore = calculateFieldScore(Field::Notes, entry->notes(), term);
            
            termScore = std::max({titleScore, usernameScore, urlScore, notesScore});
            
            // Check tags separately
            int tagIndex = entry->tagList().indexOf(term.regex);
            if (tagIndex != -1) {
                termScore = std::max(termScore, getFieldPriority(Field::Tag) * 10.0);
            }
        }

        // Handle exclusion terms
        if (term.exclude) {
            if (termScore > 0.0) {
                // If we found a match but it's excluded, reject the entry
                return 0.0;
            }
            // If we didn't find a match and it's excluded, this is good - continue
            continue;
        }

        // For non-excluded terms, we need a match
        if (termScore <= 0.0) {
            return 0.0;  // No match found for required term
        }
        
        totalScore += termScore;
        matchedTerms++;
    }

    // If no terms or all terms were excluded/skipped, return base score
    if (matchedTerms == 0) {
        return totalScore;
    }
    
    // Bonus for multiple field matches
    if (matchedTerms > 1) {
        totalScore *= (1.0 + 0.2 * (matchedTerms - 1));  // 20% bonus per additional match
    }
    
    // Bonus for non-expired entries
    if (!entry->isExpired()) {
        totalScore += 1.0;
    }

    return totalScore;
}

double EntrySearcher::calculateFieldScore(Field field, const QString& fieldValue, const SearchTerm& term)
{
    if (!term.regex.match(fieldValue).hasMatch()) {
        return 0.0;
    }
    
    double fieldPriority = getFieldPriority(field);
    double matchQuality = getMatchQuality(fieldValue, term.regex);
    
    return fieldPriority * matchQuality;
}

double EntrySearcher::getFieldPriority(Field field)
{
    switch (field) {
    case Field::Title:
        return 10.0;
    case Field::Url:
        return 8.0;
    case Field::Username:
        return 6.0;
    case Field::Notes:
        return 4.0;
    case Field::Password:
    case Field::Tag:
    case Field::Group:
        return 3.0;
    case Field::AttributeKV:
    case Field::AttributeValue:
    case Field::Attachment:
    case Field::Is:
    case Field::Has:
    case Field::Uuid:
        return 2.0;
    default:
        return 1.0;
    }
}

double EntrySearcher::getMatchQuality(const QString& fieldValue, const QRegularExpression& regex)
{
    auto match = regex.match(fieldValue);
    if (!match.hasMatch()) {
        return 0.0;
    }
    
    QString matchedText = match.captured(0);
    
    // Exact match bonus
    if (fieldValue.compare(matchedText, Qt::CaseInsensitive) == 0) {
        return 10.0;
    }
    
    // Prefix match bonus
    if (match.capturedStart(0) == 0) {
        return 5.0;
    }
    
    // Word boundary bonus (match starts at word boundary)
    if (match.capturedStart(0) == 0 || fieldValue.at(match.capturedStart(0) - 1).isSpace()) {
        return 3.0;
    }
    
    // Regular substring match
    return 1.0;
}

void EntrySearcher::parseSearchTerms(const QString& searchString)
{
    static const QList<QPair<QString, Field>> fieldnames{
        {QStringLiteral("attachment"), Field::Attachment},
        {QStringLiteral("attribute"), Field::AttributeKV},
        {QStringLiteral("notes"), Field::Notes},
        {QStringLiteral("pw"), Field::Password},
        {QStringLiteral("password"), Field::Password},
        {QStringLiteral("title"), Field::Title}, // title before tag to capture t:<word>
        {QStringLiteral("username"), Field::Username}, // username before url to capture u:<word>
        {QStringLiteral("url"), Field::Url},
        {QStringLiteral("group"), Field::Group},
        {QStringLiteral("tag"), Field::Tag},
        {QStringLiteral("is"), Field::Is},
        {QStringLiteral("has"), Field::Has},
        {QStringLiteral("uuid"), Field::Uuid}};

    // Group 1 = modifiers, Group 2 = field, Group 3 = quoted string, Group 4 = unquoted string
    static QRegularExpression termParser(R"re(([-!*+]+)?(?:(\w*):)?(?:(?=")"((?:[^"\\]|\\.)*)"|([^ ]*))( |$))re");

    m_searchTerms.clear();
    auto results = termParser.globalMatch(searchString);
    while (results.hasNext()) {
        auto result = results.next();
        SearchTerm term{};

        // Quoted string group
        term.word = result.captured(3);
        // Unescape quotes
        term.word.replace("\\\"", "\"");

        // If empty, use the unquoted string group
        if (term.word.isEmpty()) {
            term.word = result.captured(4);
        }

        // If still empty, ignore this match
        if (term.word.isEmpty()) {
            continue;
        }

        auto mods = result.captured(1);

        // Convert term to regex
        int opts = m_caseSensitive ? Tools::RegexConvertOpts::CASE_SENSITIVE : Tools::RegexConvertOpts::DEFAULT;
        if (!mods.contains("*")) {
            opts |= Tools::RegexConvertOpts::WILDCARD_ALL;
        }
        if (mods.contains("+")) {
            opts |= Tools::RegexConvertOpts::EXACT_MATCH;
        }
        term.regex = Tools::convertToRegex(term.word, opts);

        // Exclude modifier
        term.exclude = mods.contains("-") || mods.contains("!");

        // Determine the field to search
        term.field = Field::Undefined;

        QString field = result.captured(2);
        if (!field.isEmpty()) {
            if (field.startsWith("_", Qt::CaseInsensitive)) {
                term.field = Field::AttributeValue;
                // searching a custom attribute
                // in this case term.word is the attribute key (removing the leading "_")
                // and term.regex is used to match attribute value
                term.word = field.mid(1);
            } else {
                for (const auto& pair : fieldnames) {
                    if (pair.first.startsWith(field, Qt::CaseInsensitive)) {
                        term.field = pair.second;
                        break;
                    }
                }
            }
        }

        m_searchTerms.append(term);
    }
}
