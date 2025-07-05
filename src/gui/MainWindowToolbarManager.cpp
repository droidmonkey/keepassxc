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

#include "MainWindowToolbarManager.h"
#include "ui_MainWindow.h"

#include <QAction>
#include <QMenu>
#include <QToolButton>

#include "core/Config.h"
#include "core/SignalMultiplexer.h"
#include "gui/SearchWidget.h"

MainWindowToolbarManager::MainWindowToolbarManager(Ui::MainWindow* ui, SignalMultiplexer* multiplexer, QObject* parent)
    : QObject(parent)
    , m_ui(ui)
    , m_actionMultiplexer(multiplexer)
    , m_searchWidgetAction(nullptr)
    , m_showToolbarSeparator(false)
{
}

MainWindowToolbarManager::~MainWindowToolbarManager() = default;

void MainWindowToolbarManager::initializeToolbar()
{
    // Set compact mode if enabled
    if (config()->get(Config::GUI_CompactMode).toBool()) {
        m_ui->toolBar->setIconSize({20, 20});
    }

    // Setup toolbar separator visibility
    m_ui->toolbarSeparator->setVisible(false);
    m_showToolbarSeparator = config()->get(Config::GUI_ApplicationTheme).toString() != "classic";

    // Setup toolbar menus
    setupToolbarMenus();

    // Setup toolbar visibility toggle
    setupToolbarVisibilityToggle();
}

QAction* MainWindowToolbarManager::setupSearchWidget(SearchWidget* searchWidget)
{
    // Setup the search widget in the toolbar
    searchWidget->connectSignals(*m_actionMultiplexer);
    m_searchWidgetAction = m_ui->toolBar->addWidget(searchWidget);
    m_searchWidgetAction->setEnabled(false);

    // Connect search widget signals for toolbar management
    connect(searchWidget, &SearchWidget::searchCanceled, this, &MainWindowToolbarManager::onSearchWidgetCanceled);
    connect(searchWidget, &SearchWidget::lostFocus, this, &MainWindowToolbarManager::onSearchWidgetLostFocus);

    return m_searchWidgetAction;
}

void MainWindowToolbarManager::setupToolbarMenus()
{
    setupAutoTypeMenu();
    setupDatabaseLockMenu();
}

void MainWindowToolbarManager::setupAutoTypeMenu()
{
    // Build Entry Level Auto-Type menu
    auto autotypeMenu = new QMenu(qobject_cast<QWidget*>(parent()));
    autotypeMenu->addAction(m_ui->actionEntryAutoTypeSequence);
    autotypeMenu->addSeparator();
    autotypeMenu->addAction(m_ui->actionEntryAutoTypeUsername);
    autotypeMenu->addAction(m_ui->actionEntryAutoTypeUsernameEnter);
    autotypeMenu->addAction(m_ui->actionEntryAutoTypePassword);
    autotypeMenu->addAction(m_ui->actionEntryAutoTypePasswordEnter);
    autotypeMenu->addAction(m_ui->actionEntryAutoTypeTOTP);

    m_ui->actionEntryAutoType->setMenu(autotypeMenu);
    auto autoTypeButton = qobject_cast<QToolButton*>(m_ui->toolBar->widgetForAction(m_ui->actionEntryAutoType));
    if (autoTypeButton) {
        autoTypeButton->setPopupMode(QToolButton::MenuButtonPopup);
    }
}

void MainWindowToolbarManager::setupDatabaseLockMenu()
{
    auto databaseLockMenu = new QMenu(qobject_cast<QWidget*>(parent()));
    databaseLockMenu->addAction(m_ui->actionLockAllDatabases);

    m_ui->actionLockDatabaseToolbar->setMenu(databaseLockMenu);
    auto databaseLockButton =
        qobject_cast<QToolButton*>(m_ui->toolBar->widgetForAction(m_ui->actionLockDatabaseToolbar));
    if (databaseLockButton) {
        databaseLockButton->setPopupMode(QToolButton::MenuButtonPopup);
    }
}

void MainWindowToolbarManager::updateToolbarSeparatorVisibility()
{
    if (!m_showToolbarSeparator) {
        m_ui->toolbarSeparator->setVisible(false);
        return;
    }

    // Show separator when tabs are hidden and we're not in Welcome screen
    if (!m_ui->tabWidget->tabBar()->isVisible() && m_ui->stackedWidget->currentWidget() == m_ui->tabWidget) {
        m_ui->toolbarSeparator->setVisible(true);
    } else if (m_ui->stackedWidget->currentWidget() == m_ui->settingsWidget) {
        m_ui->toolbarSeparator->setVisible(true);
    } else {
        m_ui->toolbarSeparator->setVisible(false);
    }
}

void MainWindowToolbarManager::applyToolbarSettings()
{
    // Apply toolbar visibility
    m_ui->toolBar->setHidden(config()->get(Config::GUI_HideToolbar).toBool());

    // Apply movable setting
    auto movable = config()->get(Config::GUI_MovableToolbar).toBool();
    m_ui->toolBar->setMovable(movable);
    if (!movable) {
        // Move the toolbar back to the top of the main window
        if (auto mainWindow = qobject_cast<QMainWindow*>(parent())) {
            mainWindow->addToolBar(Qt::TopToolBarArea, m_ui->toolBar);
        }
    }

    // Apply tool button style
    bool isOk = false;
    const auto toolButtonStyle =
        static_cast<Qt::ToolButtonStyle>(config()->get(Config::GUI_ToolButtonStyle).toInt(&isOk));
    if (isOk) {
        m_ui->toolBar->setToolButtonStyle(toolButtonStyle);
    }
}

void MainWindowToolbarManager::setupToolbarVisibilityToggle()
{
    m_ui->actionShowToolbar->setChecked(!config()->get(Config::GUI_HideToolbar).toBool());
    connect(m_ui->actionShowToolbar, &QAction::toggled, this, &MainWindowToolbarManager::onToolbarVisibilityToggled);
}

void MainWindowToolbarManager::onSearchWidgetCanceled()
{
    m_ui->toolBar->setExpanded(false);
    updateToolbarVisibility();
}

void MainWindowToolbarManager::onSearchWidgetLostFocus()
{
    m_ui->toolBar->setExpanded(false);
    updateToolbarVisibility();
}

void MainWindowToolbarManager::onToolbarVisibilityToggled(bool checked)
{
    config()->set(Config::GUI_HideToolbar, !checked);
    // Emit a signal to request settings update from MainWindow
    if (auto mainWindow = qobject_cast<QMainWindow*>(parent())) {
        QMetaObject::invokeMethod(mainWindow, "applySettingsChanges");
    }
}

void MainWindowToolbarManager::updateToolbarVisibility()
{
    m_ui->toolBar->setVisible(!config()->get(Config::GUI_HideToolbar).toBool());
}

#include "MainWindowToolbarManager.moc"