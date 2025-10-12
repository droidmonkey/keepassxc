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

#ifndef KEEPASSX_ENTRYPAGECONTROLLER_H
#define KEEPASSX_ENTRYPAGECONTROLLER_H

#include <QObject>
#include <QWidget>

class Entry;
class Database;

/**
 * Base interface for all entry page controllers.
 * Each tab in the EditEntryWidget should have its own controller implementation.
 */
class EntryPageController : public QObject
{
    Q_OBJECT

public:
    explicit EntryPageController(QObject* parent = nullptr);
    virtual ~EntryPageController() = default;

    /**
     * Load data from the entry into the page
     */
    virtual void loadEntry(Entry* entry) = 0;

    /**
     * Save data from the page back to the entry
     * @return true if save was successful, false if validation failed
     */
    virtual bool saveEntry(Entry* entry) = 0;

    /**
     * Validate the current page data
     * @return true if all data is valid
     */
    virtual bool validateInput() = 0;

    /**
     * Clear all page data
     */
    virtual void clear() = 0;

    /**
     * Get the widget for this page
     */
    virtual QWidget* widget() = 0;

    /**
     * Get the display name for this page
     */
    virtual QString displayName() const = 0;

    /**
     * Check if this page should be enabled for the current entry/database
     */
    virtual bool isEnabled(Database* database) const = 0;

signals:
    /**
     * Emitted when the page data has been modified
     */
    void dataChanged();

    /**
     * Emitted when validation state changes
     */
    void validationChanged(bool isValid);

    /**
     * Emitted when an error occurs
     */
    void errorOccurred(const QString& message);

protected:
    Entry* m_entry = nullptr;
};

#endif // KEEPASSX_ENTRYPAGECONTROLLER_H