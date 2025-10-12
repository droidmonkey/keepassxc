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

#include "Builder.h"
#include <QUuid>

EntryTransactionBuilder Builder::createEntry(const QUuid& parentGroupId, const QUuid& entryId)
{
    EntryTransactionBuilder builder(DirectTransactionType::CreateEntry);
    builder.setTargetId(entryId.isNull() ? QUuid::createUuid() : entryId);
    builder.setParentId(parentGroupId);
    return builder;
}

EntryTransactionBuilder Builder::updateEntry(const QUuid& entryId)
{
    EntryTransactionBuilder builder(DirectTransactionType::UpdateEntry);
    builder.setTargetId(entryId);
    return builder;
}

EntryTransactionBuilder Builder::deleteEntry(const QUuid& entryId)
{
    EntryTransactionBuilder builder(DirectTransactionType::DeleteEntry);
    builder.setTargetId(entryId);
    return builder;
}

EntryTransactionBuilder Builder::moveEntry(const QUuid& entryId)
{
    EntryTransactionBuilder builder(DirectTransactionType::MoveEntry);
    builder.setTargetId(entryId);
    return builder;
}

GroupTransactionBuilder Builder::createGroup(const QUuid& parentGroupId, const QUuid& groupId)
{
    GroupTransactionBuilder builder(DirectTransactionType::CreateGroup);
    builder.setTargetId(groupId.isNull() ? QUuid::createUuid() : groupId);
    builder.setParentId(parentGroupId);
    return builder;
}

GroupTransactionBuilder Builder::updateGroup(const QUuid& groupId)
{
    GroupTransactionBuilder builder(DirectTransactionType::UpdateGroup);
    builder.setTargetId(groupId);
    return builder;
}

GroupTransactionBuilder Builder::deleteGroup(const QUuid& groupId)
{
    GroupTransactionBuilder builder(DirectTransactionType::DeleteGroup);
    builder.setTargetId(groupId);
    return builder;
}

GroupTransactionBuilder Builder::moveGroup(const QUuid& groupId)
{
    GroupTransactionBuilder builder(DirectTransactionType::MoveGroup);
    builder.setTargetId(groupId);
    return builder;
}