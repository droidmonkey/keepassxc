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

#ifndef KEEPASSX_ENTRYOPERATIONSMANAGER_H
#define KEEPASSX_ENTRYOPERATIONSMANAGER_H

#include <QList>
#include <QObject>

class DatabaseWidget;
class Entry;
class QAction;

/**
 * @brief Manages entry operations for DatabaseWidget
 *
 * This class extracts entry-specific operations (CRUD, clipboard, etc.)
 * from the DatabaseWidget to improve separation of concerns and maintainability.
 *
 * Responsibilities:
 * - Entry creation, deletion, and modification
 * - Clipboard operations (copy username, password, etc.)
 * - Entry selection and navigation
 * - TOTP operations
 * - Auto-type functionality
 */
class EntryOperationsManager : public QObject
{
    Q_OBJECT

public:
    explicit EntryOperationsManager(DatabaseWidget* parent);
    ~EntryOperationsManager() override;

    // Entry CRUD operations
    void createEntry();
    void cloneEntry();
    void deleteSelectedEntries();
    void restoreSelectedEntries();
    void expireSelectedEntries();
    void deleteEntries(QList<Entry*> entries, bool confirm = true);

    // Entry navigation and selection
    void moveEntryUp();
    void moveEntryDown();
    void focusOnEntries(bool editIfFocused = false);

    // Clipboard operations
    void copyTitle();
    void copyUsername();
    void copyPassword();
    void copyURL();
    void copyNotes();
    void copyAttribute(QAction* action);
    bool copyFocusedTextSelection();

    // TOTP operations
    void showTotp();
    void showTotpKeyQrCode();
    void copyTotp();
    void copyPasswordTotp();
    void setupTotp();

    // Auto-type operations
    void performAutoType(const QString& sequence = {});
    void performAutoTypeUsername();
    void performAutoTypeUsernameEnter();
    void performAutoTypePassword();
    void performAutoTypePasswordEnter();
    void performAutoTypeTOTP();

    // Utility operations
    void setClipboardTextAndMinimize(const QString& text);
    void openUrl();
    void openUrlForEntry(Entry* entry);

    // Icon and favicon operations
    void downloadSelectedFavicons();
    void downloadAllFavicons();
    void downloadFaviconInBackground(Entry* entry);

signals:
    /**
     * @brief Emitted when an entry operation is completed
     * @param operation description of the operation performed
     * @param success true if operation was successful
     */
    void entryOperationCompleted(const QString& operation, bool success);

    /**
     * @brief Emitted when clipboard content is set
     * @param content description of what was copied
     */
    void clipboardContentSet(const QString& content);

private:
    /**
     * @brief Get the currently selected entries
     * @return list of selected entries
     */
    QList<Entry*> getSelectedEntries() const;

    /**
     * @brief Get the currently focused entry
     * @return pointer to focused entry or nullptr
     */
    Entry* getCurrentEntry() const;

    /**
     * @brief Perform icon downloads for specified entries
     * @param entries list of entries to download icons for
     * @param force force download even if icon exists
     * @param downloadInBackground perform download in background
     */
    void performIconDownloads(const QList<Entry*>& entries, bool force = false, bool downloadInBackground = false);

    DatabaseWidget* m_databaseWidget; ///< Reference to parent DatabaseWidget

    Q_DISABLE_COPY(EntryOperationsManager)
};

#endif // KEEPASSX_ENTRYOPERATIONSMANAGER_H