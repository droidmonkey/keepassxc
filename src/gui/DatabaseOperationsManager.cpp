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

#include "DatabaseOperationsManager.h"
#include "DatabaseWidget.h"

#include <QApplication>
#include <QFileInfo>
#include <QPointer>

#include "core/Config.h"
#include "gui/FileDialog.h"
#include "gui/MainWindow.h"
#include "gui/MessageBox.h"

DatabaseOperationsManager::DatabaseOperationsManager(DatabaseWidget* parent)
    : QObject(parent)
    , m_databaseWidget(parent)
    , m_saveAttempts(0)
    , m_isSaving(false)
{
    Q_ASSERT(m_databaseWidget);
}

DatabaseOperationsManager::~DatabaseOperationsManager() = default;

bool DatabaseOperationsManager::save()
{
    if (!m_databaseWidget->database()) {
        return false;
    }

    if (m_isSaving) {
        return false;
    }

    m_isSaving = true;
    emit saveStarted();

    QString errorMessage;
    bool success = performSave(errorMessage);

    if (!success && !errorMessage.isEmpty()) {
        m_databaseWidget->showErrorMessage(errorMessage);
    }

    m_isSaving = false;
    emit saveCompleted(success);

    return success;
}

bool DatabaseOperationsManager::saveAs()
{
    if (!m_databaseWidget->database()) {
        return false;
    }

    auto db = m_databaseWidget->database();
    auto oldFileName = db->filePath();

    QString fileName = fileDialog()->getSaveFileName(m_databaseWidget,
                                                     tr("Save Database"),
                                                     oldFileName.isEmpty() ? tr("Passwords") : oldFileName,
                                                     tr("KeePass Database (*.kdbx)"));

    if (fileName.isEmpty()) {
        return false;
    }

    if (m_isSaving) {
        return false;
    }

    m_isSaving = true;
    emit saveStarted();

    QString errorMessage;
    bool success = performSave(errorMessage, fileName);

    if (!success && !errorMessage.isEmpty()) {
        m_databaseWidget->showErrorMessage(errorMessage);
    }

    m_isSaving = false;
    emit saveCompleted(success);

    return success;
}

bool DatabaseOperationsManager::saveBackup()
{
    if (!m_databaseWidget->database()) {
        return false;
    }

    auto db = m_databaseWidget->database();
    auto fileName = db->filePath();

    if (fileName.isEmpty()) {
        m_databaseWidget->showErrorMessage(tr("Database must be saved first"));
        return false;
    }

    QFileInfo dbFileInfo(fileName);
    QString backupFileName = QString("%1/%2_backup_%3.%4")
                                 .arg(dbFileInfo.absolutePath())
                                 .arg(dbFileInfo.baseName())
                                 .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"))
                                 .arg(dbFileInfo.completeSuffix());

    QString errorMessage;
    bool success = performSave(errorMessage, backupFileName);

    if (!success && !errorMessage.isEmpty()) {
        m_databaseWidget->showErrorMessage(tr("Failed to create backup: %1").arg(errorMessage));
    } else if (success) {
        m_databaseWidget->showMessage(tr("Backup created successfully: %1").arg(backupFileName),
                                      MessageWidget::Information);
    }

    return success;
}

bool DatabaseOperationsManager::lock()
{
    if (!m_databaseWidget->database()) {
        return false;
    }

    auto db = m_databaseWidget->database();

    if (db->isModified()) {
        auto result = MessageBox::question(m_databaseWidget,
                                           tr("Lock Database"),
                                           tr("The database has unsaved changes. Save before locking?"),
                                           MessageBox::Save | MessageBox::Discard | MessageBox::Cancel,
                                           MessageBox::Save);

        if (result == MessageBox::Cancel) {
            return false;
        }

        if (result == MessageBox::Save) {
            if (!save()) {
                return false;
            }
        }
    }

    // Signal that the database is about to be locked
    emit m_databaseWidget->databaseLockRequested();

    // Switch to locked mode through the parent widget
    m_databaseWidget->switchToOpenDatabase();

    emit lockCompleted(true);
    return true;
}

bool DatabaseOperationsManager::isSaving() const
{
    return m_isSaving;
}

void DatabaseOperationsManager::onAutosaveDelayTimeout()
{
    // Delegate to the main save operation
    save();
}

bool DatabaseOperationsManager::performSave(QString& errorMessage, const QString& fileName)
{
    auto db = m_databaseWidget->database();
    if (!db) {
        errorMessage = tr("No database loaded");
        return false;
    }

    QPointer<QWidget> focusWidget(qApp->focusWidget());

    // Lock out interactions during save
    auto mainWindow = m_databaseWidget->findChild<QWidget*>("MainWindow");
    if (mainWindow) {
        mainWindow->setDisabled(true);
    }
    QApplication::processEvents();

    Database::SaveAction saveAction = Database::Atomic;
    if (!config()->get(Config::UseAtomicSaves).toBool()) {
        if (config()->get(Config::UseDirectWriteSaves).toBool()) {
            saveAction = Database::DirectWrite;
        } else {
            saveAction = Database::TempFile;
        }
    }

    QString backupFilePath;
    if (config()->get(Config::BackupBeforeSave).toBool()) {
        backupFilePath = config()->get(Config::BackupFilePathPattern).toString();
        // Fall back to default
        if (backupFilePath.isEmpty()) {
            backupFilePath = config()->getDefault(Config::BackupFilePathPattern).toString();
        }

        QFileInfo dbFileInfo(db->filePath());
        backupFilePath.replace("{DB_FILENAME}", dbFileInfo.baseName());
        backupFilePath.replace("{YYYY}", QString::number(QDateTime::currentDateTime().date().year()));
        backupFilePath.replace("{MM}",
                               QString::number(QDateTime::currentDateTime().date().month()).rightJustified(2, '0'));
        backupFilePath.replace("{DD}",
                               QString::number(QDateTime::currentDateTime().date().day()).rightJustified(2, '0'));
        backupFilePath.replace("{hh}",
                               QString::number(QDateTime::currentDateTime().time().hour()).rightJustified(2, '0'));
        backupFilePath.replace("{mm}",
                               QString::number(QDateTime::currentDateTime().time().minute()).rightJustified(2, '0'));
        backupFilePath.replace("{ss}",
                               QString::number(QDateTime::currentDateTime().time().second()).rightJustified(2, '0'));
    }

    bool success = false;
    const QString saveFileName = fileName.isEmpty() ? db->filePath() : fileName;

    if (!fileName.isEmpty()) {
        success = db->saveAs(saveFileName, saveAction, backupFilePath, &errorMessage);
    } else {
        success = db->save(saveAction, backupFilePath, &errorMessage);
    }

    // Re-enable interactions
    if (mainWindow) {
        mainWindow->setDisabled(false);
    }

    // Restore focus
    if (focusWidget) {
        focusWidget->setFocus();
    }

    if (success) {
        m_saveAttempts = 0;
    } else {
        ++m_saveAttempts;
    }

    return success;
}
