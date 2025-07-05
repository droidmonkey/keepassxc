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

#ifndef KEEPASSX_ENTRYDATACOORDINATOR_H
#define KEEPASSX_ENTRYDATACOORDINATOR_H

#include <QObject>
#include <QList>
#include <QPointer>

class Entry;
class Database;
class EntryPageController;

/**
 * Coordinates data sharing and synchronization between entry page controllers.
 * Manages the overall state of entry editing and ensures consistency across pages.
 */
class EntryDataCoordinator : public QObject
{
    Q_OBJECT

public:
    explicit EntryDataCoordinator(QObject* parent = nullptr);
    ~EntryDataCoordinator() override;

    /**
     * Register a page controller with the coordinator
     */
    void registerPageController(EntryPageController* controller);

    /**
     * Remove a page controller from the coordinator
     */
    void unregisterPageController(EntryPageController* controller);

    /**
     * Load an entry into all registered page controllers
     */
    void loadEntry(Entry* entry);

    /**
     * Save data from all page controllers back to the entry
     * @return true if all controllers saved successfully
     */
    bool saveEntry();

    /**
     * Validate all page controllers
     * @return true if all controllers are valid
     */
    bool validateAll();

    /**
     * Clear all page controllers
     */
    void clearAll();

    /**
     * Get the current entry being edited
     */
    Entry* currentEntry() const;

    /**
     * Check if any page controller has unsaved changes
     */
    bool hasUnsavedChanges() const;

    /**
     * Get list of enabled page controllers for the current database
     */
    QList<EntryPageController*> enabledControllers(Database* database) const;

signals:
    /**
     * Emitted when any page data changes
     */
    void dataChanged();

    /**
     * Emitted when overall validation state changes
     */
    void validationChanged(bool isValid);

    /**
     * Emitted when an error occurs in any controller
     */
    void errorOccurred(const QString& message);

private slots:
    void onControllerDataChanged();
    void onControllerValidationChanged(bool isValid);
    void onControllerErrorOccurred(const QString& message);

private:
    void updateValidationState();

    QList<QPointer<EntryPageController>> m_controllers;
    QPointer<Entry> m_currentEntry;
    bool m_hasUnsavedChanges = false;
    bool m_isValid = true;
};

#endif // KEEPASSX_ENTRYDATACOORDINATOR_H