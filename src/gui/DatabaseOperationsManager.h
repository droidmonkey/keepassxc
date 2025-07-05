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

#ifndef KEEPASSX_DATABASEOPERATIONSMANAGER_H
#define KEEPASSX_DATABASEOPERATIONSMANAGER_H

#include <QObject>
#include <QSharedPointer>

#include "core/Database.h"

class DatabaseWidget;

/**
 * @brief Manages database operations for DatabaseWidget
 * 
 * This class extracts database-specific operations (save, load, lock, unlock)
 * from the DatabaseWidget to improve separation of concerns and maintainability.
 * 
 * Responsibilities:
 * - Database saving operations (save, saveAs, saveBackup)
 * - Database locking/unlocking
 * - Database reloading and merging
 * - File operation coordination
 */
class DatabaseOperationsManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseOperationsManager(DatabaseWidget* parent);
    ~DatabaseOperationsManager() override;

    /**
     * @brief Save the database
     * @return true if save was successful
     */
    bool save();

    /**
     * @brief Save the database with a new filename
     * @return true if save was successful
     */
    bool saveAs();

    /**
     * @brief Create a backup of the database
     * @return true if backup was successful
     */
    bool saveBackup();

    /**
     * @brief Lock the database
     * @return true if lock was successful
     */
    bool lock();

    /**
     * @brief Check if database is currently being saved
     * @return true if save operation is in progress
     */
    bool isSaving() const;

signals:
    /**
     * @brief Emitted when database save operation starts
     */
    void saveStarted();

    /**
     * @brief Emitted when database save operation completes
     * @param success true if save was successful
     */
    void saveCompleted(bool success);

    /**
     * @brief Emitted when database lock operation completes
     * @param success true if lock was successful
     */
    void lockCompleted(bool success);

private slots:
    /**
     * @brief Handle autosave timer timeout
     */
    void onAutosaveDelayTimeout();

private:
    /**
     * @brief Perform the actual save operation
     * @param errorMessage output parameter for error details
     * @param fileName optional specific filename to save to
     * @return true if save was successful
     */
    bool performSave(QString& errorMessage, const QString& fileName = {});

    DatabaseWidget* m_databaseWidget; ///< Reference to parent DatabaseWidget
    int m_saveAttempts;               ///< Number of save attempts for current operation
    bool m_isSaving;                  ///< Flag indicating save operation in progress

    Q_DISABLE_COPY(DatabaseOperationsManager)
};

#endif // KEEPASSX_DATABASEOPERATIONSMANAGER_H