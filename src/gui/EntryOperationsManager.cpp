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

#include "EntryOperationsManager.h"
#include "DatabaseWidget.h"

#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>

#include "autotype/AutoType.h"
#include "core/Entry.h"
#include "core/Group.h"
#include "core/Tools.h"
#include "gui/Clipboard.h"
#include "gui/GuiTools.h"
#include "gui/MessageBox.h"
#include "gui/TotpDialog.h"
#include "gui/TotpSetupDialog.h"
#include "gui/entry/EntryView.h"

#ifdef WITH_XC_SSHAGENT
#include "sshagent/SSHAgent.h"
#endif

#ifdef WITH_XC_NETWORKING
#include "gui/IconDownloaderDialog.h"
#endif

EntryOperationsManager::EntryOperationsManager(DatabaseWidget* parent)
    : QObject(parent)
    , m_databaseWidget(parent)
{
    Q_ASSERT(m_databaseWidget);
}

EntryOperationsManager::~EntryOperationsManager() = default;

void EntryOperationsManager::createEntry()
{
    if (!m_databaseWidget->currentGroup()) {
        return;
    }

    auto entry = new Entry();
    entry->setUuid(QUuid::createUuid());
    entry->setGroup(m_databaseWidget->currentGroup());
    
    // Switch to entry edit mode
    m_databaseWidget->switchToEntryEdit(entry, true);
    
    emit entryOperationCompleted(tr("Entry created"), true);
}

void EntryOperationsManager::cloneEntry()
{
    auto currentEntry = getCurrentEntry();
    if (!currentEntry) {
        return;
    }

    auto clonedEntry = currentEntry->clone(Entry::CloneNewUuid);
    clonedEntry->setGroup(currentEntry->group());
    
    // Switch to entry edit mode for the cloned entry
    m_databaseWidget->switchToEntryEdit(clonedEntry, true);
    
    emit entryOperationCompleted(tr("Entry cloned"), true);
}

void EntryOperationsManager::deleteSelectedEntries()
{
    auto selectedEntries = getSelectedEntries();
    if (selectedEntries.isEmpty()) {
        return;
    }

    deleteEntries(selectedEntries);
}

void EntryOperationsManager::restoreSelectedEntries()
{
    auto selectedEntries = getSelectedEntries();
    if (selectedEntries.isEmpty()) {
        return;
    }

    for (auto entry : selectedEntries) {
        if (entry->isRecycled()) {
            // Move entry back to root group or its original location
            auto rootGroup = m_databaseWidget->database()->rootGroup();
            entry->setGroup(rootGroup);
        }
    }
    
    emit entryOperationCompleted(tr("Entries restored"), true);
}

void EntryOperationsManager::expireSelectedEntries()
{
    auto selectedEntries = getSelectedEntries();
    if (selectedEntries.isEmpty()) {
        return;
    }

    for (auto entry : selectedEntries) {
        entry->setExpiryTime(QDateTime::currentDateTime());
    }
    
    emit entryOperationCompleted(tr("Entries expired"), true);
}

void EntryOperationsManager::deleteEntries(QList<Entry*> entries, bool confirm)
{
    if (entries.isEmpty()) {
        return;
    }

    if (confirm) {
        auto result = MessageBox::question(
            m_databaseWidget,
            tr("Delete Entries"),
            tr("Are you sure you want to delete %1 entries?").arg(entries.size()),
            MessageBox::Delete | MessageBox::Cancel,
            MessageBox::Cancel);

        if (result != MessageBox::Delete) {
            return;
        }
    }

    GuiTools::deleteEntriesResolveReferences(m_databaseWidget, entries, confirm);
    
    emit entryOperationCompleted(tr("Entries deleted"), true);
}

void EntryOperationsManager::moveEntryUp()
{
    auto currentEntry = getCurrentEntry();
    if (!currentEntry) {
        return;
    }

    // Move entry up in the entry view by changing selection
    auto entryView = m_databaseWidget->entryView();
    if (entryView) {
        auto currentIndex = entryView->currentIndex();
        if (currentIndex.isValid() && currentIndex.row() > 0) {
            auto newIndex = entryView->model()->index(currentIndex.row() - 1, currentIndex.column());
            entryView->setCurrentIndex(newIndex);
        }
    }
}

void EntryOperationsManager::moveEntryDown()
{
    auto currentEntry = getCurrentEntry();
    if (!currentEntry) {
        return;
    }

    // Move entry down in the entry view by changing selection
    auto entryView = m_databaseWidget->entryView();
    if (entryView) {
        auto currentIndex = entryView->currentIndex();
        if (currentIndex.isValid() && currentIndex.row() < entryView->model()->rowCount() - 1) {
            auto newIndex = entryView->model()->index(currentIndex.row() + 1, currentIndex.column());
            entryView->setCurrentIndex(newIndex);
        }
    }
}

void EntryOperationsManager::focusOnEntries(bool editIfFocused)
{
    auto entryView = m_databaseWidget->entryView();
    if (entryView) {
        entryView->setFocus();
        if (editIfFocused && getCurrentEntry()) {
            m_databaseWidget->switchToEntryEdit();
        }
    }
}

void EntryOperationsManager::copyTitle()
{
    auto currentEntry = getCurrentEntry();
    if (!currentEntry) {
        return;
    }

    setClipboardTextAndMinimize(currentEntry->title());
    emit clipboardContentSet(tr("Title"));
}

void EntryOperationsManager::copyUsername()
{
    auto currentEntry = getCurrentEntry();
    if (!currentEntry) {
        return;
    }

    setClipboardTextAndMinimize(currentEntry->username());
    emit clipboardContentSet(tr("Username"));
}

void EntryOperationsManager::copyPassword()
{
    auto currentEntry = getCurrentEntry();
    if (!currentEntry) {
        return;
    }

    setClipboardTextAndMinimize(currentEntry->password());
    emit clipboardContentSet(tr("Password"));
}

void EntryOperationsManager::copyURL()
{
    auto currentEntry = getCurrentEntry();
    if (!currentEntry) {
        return;
    }

    setClipboardTextAndMinimize(currentEntry->url());
    emit clipboardContentSet(tr("URL"));
}

void EntryOperationsManager::copyNotes()
{
    auto currentEntry = getCurrentEntry();
    if (!currentEntry) {
        return;
    }

    setClipboardTextAndMinimize(currentEntry->notes());
    emit clipboardContentSet(tr("Notes"));
}

void EntryOperationsManager::copyAttribute(QAction* action)
{
    if (!action) {
        return;
    }

    auto currentEntry = getCurrentEntry();
    if (!currentEntry) {
        return;
    }

    QString attributeKey = action->data().toString();
    QString attributeValue = currentEntry->attributes()->value(attributeKey);
    
    setClipboardTextAndMinimize(attributeValue);
    emit clipboardContentSet(tr("Custom attribute: %1").arg(attributeKey));
}

bool EntryOperationsManager::copyFocusedTextSelection()
{
    // This would need to be implemented based on the current focus widget
    // For now, return false to indicate not implemented
    return false;
}

void EntryOperationsManager::showTotp()
{
    auto currentEntry = getCurrentEntry();
    if (!currentEntry || !currentEntry->hasTotp()) {
        return;
    }

    auto totpDialog = new TotpDialog(m_databaseWidget, currentEntry);
    totpDialog->open();
}

void EntryOperationsManager::showTotpKeyQrCode()
{
    auto currentEntry = getCurrentEntry();
    if (!currentEntry || !currentEntry->hasTotp()) {
        return;
    }

    // Implementation would show QR code for TOTP setup
    // This is a placeholder for the actual implementation
}

void EntryOperationsManager::copyTotp()
{
    auto currentEntry = getCurrentEntry();
    if (!currentEntry || !currentEntry->hasTotp()) {
        return;
    }

    QString totpCode = currentEntry->totp();
    setClipboardTextAndMinimize(totpCode);
    emit clipboardContentSet(tr("TOTP code"));
}

void EntryOperationsManager::copyPasswordTotp()
{
    auto currentEntry = getCurrentEntry();
    if (!currentEntry) {
        return;
    }

    QString combined = currentEntry->password();
    if (currentEntry->hasTotp()) {
        combined += currentEntry->totp();
    }
    
    setClipboardTextAndMinimize(combined);
    emit clipboardContentSet(tr("Password with TOTP"));
}

void EntryOperationsManager::setupTotp()
{
    auto currentEntry = getCurrentEntry();
    if (!currentEntry) {
        return;
    }

    auto setupDialog = new TotpSetupDialog(m_databaseWidget, currentEntry);
    setupDialog->open();
}

void EntryOperationsManager::performAutoType(const QString& sequence)
{
    Q_UNUSED(sequence) // For now, ignore the sequence parameter
    
    auto currentEntry = getCurrentEntry();
    if (!currentEntry) {
        return;
    }

    autoType()->performAutoType(currentEntry);
}

void EntryOperationsManager::performAutoTypeUsername()
{
    performAutoType("{USERNAME}");
}

void EntryOperationsManager::performAutoTypeUsernameEnter()
{
    performAutoType("{USERNAME}{ENTER}");
}

void EntryOperationsManager::performAutoTypePassword()
{
    performAutoType("{PASSWORD}");
}

void EntryOperationsManager::performAutoTypePasswordEnter()
{
    performAutoType("{PASSWORD}{ENTER}");
}

void EntryOperationsManager::performAutoTypeTOTP()
{
    performAutoType("{TOTP}");
}

void EntryOperationsManager::setClipboardTextAndMinimize(const QString& text)
{
    clipboard()->setText(text);
    
    // Minimize main window if configured
    auto mainWindow = m_databaseWidget->findChild<QWidget*>("MainWindow");
    if (mainWindow) {
        mainWindow->showMinimized();
    }
}

void EntryOperationsManager::openUrl()
{
    auto currentEntry = getCurrentEntry();
    if (currentEntry) {
        openUrlForEntry(currentEntry);
    }
}

void EntryOperationsManager::openUrlForEntry(Entry* entry)
{
    if (!entry) {
        return;
    }

    QString url = entry->url();
    if (!url.isEmpty()) {
        QDesktopServices::openUrl(QUrl(url));
    }
}



void EntryOperationsManager::downloadSelectedFavicons()
{
    auto selectedEntries = getSelectedEntries();
    if (selectedEntries.isEmpty()) {
        return;
    }

    performIconDownloads(selectedEntries, true);
}

void EntryOperationsManager::downloadAllFavicons()
{
    if (!m_databaseWidget->database()) {
        return;
    }

    auto allEntries = m_databaseWidget->database()->rootGroup()->entriesRecursive();
    performIconDownloads(allEntries, true);
}

void EntryOperationsManager::downloadFaviconInBackground(Entry* entry)
{
    if (!entry) {
        return;
    }

    performIconDownloads({entry}, false, true);
}

QList<Entry*> EntryOperationsManager::getSelectedEntries() const
{
    auto entryView = m_databaseWidget->entryView();
    if (!entryView) {
        return {};
    }

    return entryView->selectedEntries();
}

Entry* EntryOperationsManager::getCurrentEntry() const
{
    return m_databaseWidget->currentSelectedEntry();
}

void EntryOperationsManager::performIconDownloads(const QList<Entry*>& entries, bool force, bool downloadInBackground)
{
#ifdef WITH_XC_NETWORKING
    if (downloadInBackground) {
        // Implementation for background download
        // This would involve creating a separate worker thread
    } else {
        auto iconDownloader = new IconDownloaderDialog(m_databaseWidget);
        iconDownloader->downloadFavicons(m_databaseWidget->database(), entries, force);
    }
#else
    Q_UNUSED(entries)
    Q_UNUSED(force)
    Q_UNUSED(downloadInBackground)
#endif
}

