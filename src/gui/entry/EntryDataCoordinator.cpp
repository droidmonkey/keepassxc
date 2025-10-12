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

#include "EntryDataCoordinator.h"
#include "EntryPageController.h"
#include "core/Entry.h"

EntryDataCoordinator::EntryDataCoordinator(QObject* parent)
    : QObject(parent)
{
}

EntryDataCoordinator::~EntryDataCoordinator() = default;

void EntryDataCoordinator::registerPageController(EntryPageController* controller)
{
    if (!controller || m_controllers.contains(controller)) {
        return;
    }

    m_controllers.append(controller);

    // Connect signals to coordinate changes
    connect(controller, &EntryPageController::dataChanged,
            this, &EntryDataCoordinator::onControllerDataChanged);
    connect(controller, &EntryPageController::validationChanged,
            this, &EntryDataCoordinator::onControllerValidationChanged);
    connect(controller, &EntryPageController::errorOccurred,
            this, &EntryDataCoordinator::onControllerErrorOccurred);
}

void EntryDataCoordinator::unregisterPageController(EntryPageController* controller)
{
    if (!controller) {
        return;
    }

    m_controllers.removeOne(controller);
    controller->disconnect(this);
}

void EntryDataCoordinator::loadEntry(Entry* entry)
{
    m_currentEntry = entry;
    m_hasUnsavedChanges = false;

    for (auto controller : m_controllers) {
        if (controller) {
            controller->loadEntry(entry);
        }
    }

    updateValidationState();
}

bool EntryDataCoordinator::saveEntry()
{
    if (!m_currentEntry) {
        return false;
    }

    bool allSaved = true;
    for (auto controller : m_controllers) {
        if (controller && !controller->saveEntry(m_currentEntry)) {
            allSaved = false;
        }
    }

    if (allSaved) {
        m_hasUnsavedChanges = false;
    }

    return allSaved;
}

bool EntryDataCoordinator::validateAll()
{
    bool allValid = true;
    for (auto controller : m_controllers) {
        if (controller && !controller->validateInput()) {
            allValid = false;
        }
    }

    return allValid;
}

void EntryDataCoordinator::clearAll()
{
    for (auto controller : m_controllers) {
        if (controller) {
            controller->clear();
        }
    }

    m_currentEntry = nullptr;
    m_hasUnsavedChanges = false;
    m_isValid = true;
}

Entry* EntryDataCoordinator::currentEntry() const
{
    return m_currentEntry;
}

bool EntryDataCoordinator::hasUnsavedChanges() const
{
    return m_hasUnsavedChanges;
}

QList<EntryPageController*> EntryDataCoordinator::enabledControllers(Database* database) const
{
    QList<EntryPageController*> enabled;
    for (auto controller : m_controllers) {
        if (controller && controller->isEnabled(database)) {
            enabled.append(controller);
        }
    }
    return enabled;
}

void EntryDataCoordinator::onControllerDataChanged()
{
    m_hasUnsavedChanges = true;
    emit dataChanged();
}

void EntryDataCoordinator::onControllerValidationChanged(bool isValid)
{
    Q_UNUSED(isValid)
    updateValidationState();
}

void EntryDataCoordinator::onControllerErrorOccurred(const QString& message)
{
    emit errorOccurred(message);
}

void EntryDataCoordinator::updateValidationState()
{
    bool newValidState = validateAll();
    if (newValidState != m_isValid) {
        m_isValid = newValidState;
        emit validationChanged(m_isValid);
    }
}