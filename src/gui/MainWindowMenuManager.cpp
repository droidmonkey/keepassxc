/*
 *  Copyright (C) 2024 KeePassXC Team <team@keepassxc.org>
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

#include "MainWindowMenuManager.h"
#include "ui_MainWindow.h"

#include <QActionGroup>
#include <QMenu>

#include "core/Config.h"
#include "gui/DatabaseTabWidget.h"
#include "gui/DatabaseWidget.h"
#include "gui/Icons.h"
#include "gui/entry/EntryView.h"

MainWindowMenuManager::MainWindowMenuManager(Ui::MainWindow* ui, DatabaseTabWidget* tabWidget, QObject* parent)
    : QObject(parent)
    , m_ui(ui)
    , m_tabWidget(tabWidget)
    , m_entryContextMenu(nullptr)
    , m_entryNewContextMenu(nullptr)
    , m_lastDatabasesActions(nullptr)
    , m_copyAdditionalAttributeActions(nullptr)
    , m_setTagsMenuActions(nullptr)
    , m_clearHistoryAction(nullptr)
    , m_countDefaultAttributes(0)
{
}

MainWindowMenuManager::~MainWindowMenuManager() = default;

void MainWindowMenuManager::initializeMenus()
{
    setupMenuActionGroups();
    createEntryContextMenu();
    createEntryNewContextMenu();

    // Store count of default attributes for later use
    m_countDefaultAttributes = m_ui->menuEntryCopyAttribute->actions().size();
}

void MainWindowMenuManager::setupMenuActionGroups()
{
    // Create action groups for menu management
    m_lastDatabasesActions = new QActionGroup(this);
    connect(m_lastDatabasesActions, &QActionGroup::triggered, this, &MainWindowMenuManager::onOpenRecentDatabase);

    m_copyAdditionalAttributeActions = new QActionGroup(this);

    m_setTagsMenuActions = new QActionGroup(this);

    // Create clear history action
    m_clearHistoryAction = new QAction(tr("Clear History"), this);
    connect(m_clearHistoryAction, &QAction::triggered, this, &MainWindowMenuManager::onClearLastDatabases);
}

void MainWindowMenuManager::createEntryContextMenu()
{
    m_entryContextMenu = new QMenu(qobject_cast<QWidget*>(parent()));
    m_entryContextMenu->setSeparatorsCollapsible(true);
    m_entryContextMenu->addAction(m_ui->actionEntryRestore);
    m_entryContextMenu->addSeparator();
    m_entryContextMenu->addAction(m_ui->actionEntryCopyUsername);
    m_entryContextMenu->addAction(m_ui->actionEntryCopyPassword);
    m_entryContextMenu->addAction(m_ui->actionEntryCopyURL);
    m_entryContextMenu->addAction(m_ui->menuEntryCopyAttribute->menuAction());
    m_entryContextMenu->addAction(m_ui->menuEntryTotp->menuAction());
    m_entryContextMenu->addAction(m_ui->menuTags->menuAction());
    m_entryContextMenu->addSeparator();
    m_entryContextMenu->addAction(m_ui->actionEntryAutoType);
    m_entryContextMenu->addSeparator();
#ifdef WITH_XC_BROWSER_PASSKEYS
    m_entryContextMenu->addAction(m_ui->actionEntryImportPasskey);
    m_entryContextMenu->addAction(m_ui->actionEntryRemovePasskey);
    m_entryContextMenu->addSeparator();
#endif
    m_entryContextMenu->addAction(m_ui->actionEntryEdit);
    m_entryContextMenu->addAction(m_ui->actionEntryExpire);
    m_entryContextMenu->addAction(m_ui->actionEntryClone);
    m_entryContextMenu->addAction(m_ui->actionEntryDelete);
    m_entryContextMenu->addAction(m_ui->actionEntryNew);
    m_entryContextMenu->addSeparator();
    m_entryContextMenu->addAction(m_ui->actionEntryMoveUp);
    m_entryContextMenu->addAction(m_ui->actionEntryMoveDown);
    m_entryContextMenu->addSeparator();
    m_entryContextMenu->addAction(m_ui->actionEntryOpenUrl);
    m_entryContextMenu->addAction(m_ui->actionEntryDownloadIcon);
    m_entryContextMenu->addSeparator();
    m_entryContextMenu->addAction(m_ui->actionEntryAddToAgent);
    m_entryContextMenu->addAction(m_ui->actionEntryRemoveFromAgent);
}

void MainWindowMenuManager::createEntryNewContextMenu()
{
    m_entryNewContextMenu = new QMenu(qobject_cast<QWidget*>(parent()));
    m_entryNewContextMenu->addAction(m_ui->actionEntryNew);
}

void MainWindowMenuManager::updateLastDatabasesMenu()
{
    m_ui->menuRecentDatabases->clear();

    const QStringList lastDatabases = config()->get(Config::LastDatabases).toStringList();
    for (const QString& database : lastDatabases) {
        QAction* action = m_ui->menuRecentDatabases->addAction(database);
        action->setData(database);
        m_lastDatabasesActions->addAction(action);
    }
    m_ui->menuRecentDatabases->addSeparator();
    m_ui->menuRecentDatabases->addAction(m_clearHistoryAction);
}

void MainWindowMenuManager::updateCopyAttributesMenu()
{
    DatabaseWidget* dbWidget = m_tabWidget->currentDatabaseWidget();
    if (!dbWidget) {
        return;
    }

    if (dbWidget->numberOfSelectedEntries() != 1) {
        return;
    }

    QList<QAction*> actions = m_ui->menuEntryCopyAttribute->actions();
    for (int i = m_countDefaultAttributes; i < actions.size(); i++) {
        delete actions[i];
    }

    const QStringList customEntryAttributes = dbWidget->customEntryAttributes();
    for (const QString& key : customEntryAttributes) {
        QAction* action = m_ui->menuEntryCopyAttribute->addAction(key);
        action->setData(QVariant(key));
        m_copyAdditionalAttributeActions->addAction(action);
    }
}

void MainWindowMenuManager::updateSetTagsMenu()
{
    auto actionForTag = [](const QMenu* menu, const QString& tag) -> QAction* {
        for (const auto action : menu->actions()) {
            if (action->text() == tag) {
                return action;
            }
        }
        return nullptr;
    };

    m_ui->menuTags->setTearOffEnabled(true);

    auto dbWidget = m_tabWidget->currentDatabaseWidget();
    if (dbWidget) {
        // Enumerate tags applied to the selected entries
        QSet<QString> selectedTags;
        for (const auto entry : dbWidget->entryView()->selectedEntries()) {
            for (const auto& tag : entry->tagList()) {
                selectedTags.insert(tag);
            }
        }

        // Remove missing tags
        const auto tagList = dbWidget->database()->tagList();
        for (const auto action : m_ui->menuTags->actions()) {
            if (!tagList.contains(action->text()) || !action->isEnabled()) {
                delete action;
            }
        }

        // Add known database tags as actions and set checked if
        // a selected entry has that tag
        for (const auto& tag : tagList) {
            auto action = actionForTag(m_ui->menuTags, tag);
            if (!action) {
                action = m_ui->menuTags->addAction(icons()->icon("tag"), tag);
                action->setCheckable(true);
                m_setTagsMenuActions->addAction(action);
            }
            action->setChecked(selectedTags.contains(tag));
        }
    }

    // If no tags exist in the database then show a tip to the user
    if (m_ui->menuTags->isEmpty()) {
        m_ui->menuTags->setTearOffEnabled(false);
        auto action = m_ui->menuTags->addAction(tr("No Tags"));
        action->setEnabled(false);
    }
}

void MainWindowMenuManager::onOpenRecentDatabase(QAction* action)
{
    emit openRecentDatabase(action->data().toString());
}

void MainWindowMenuManager::onClearLastDatabases()
{
    emit clearLastDatabases();
}

#include "MainWindowMenuManager.moc"