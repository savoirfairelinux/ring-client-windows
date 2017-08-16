/***************************************************************************
 * Copyright (C) 2017 by Savoir-faire Linux                                *
 * Author: Anthony Léonard <anthony.leonard@savoirfairelinux.com>          *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 3 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 **************************************************************************/
// Qt
#include <QDateTime>

// LRC
#include "globalinstances.h"
#include "api/contactmodel.h"
#include "api/conversationmodel.h"

// Client
#include "smartlistmodel.h"
#include "pixbufmanipulator.h"

SmartListModel::SmartListModel(AccountInfo &acc, QObject *parent)
    : QAbstractItemModel(parent),
      acc_(acc)
{}

int SmartListModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return acc_.conversationModel->getFilteredConversations().size();

    return 0; // A valid QModelIndex returns 0 as no entry has sub-elements
}

int SmartListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 1;
}

QVariant SmartListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const auto* item = &acc_.conversationModel->getFilteredConversations().at(index.row());

    if (item) {
        switch (role) {
        case Role::Picture:
        case Qt::DecorationRole:
            return GlobalInstances::pixmapManipulator().decorationRole(index);
        case Role::DisplayName:
        case Qt::DisplayRole:
        {
            // TODO Handle conversations with multiple contacts
            auto& contact = acc_.contactModel->getContact(item->participants[0]);
            return QVariant(QString::fromStdString(contact.alias));
        }
        case Role::DisplayID:
        {
            // TODO Handle conversations with multiple contacts
            auto& contact = acc_.contactModel->getContact(item->participants[0]);
            if (!contact.registeredName.empty())
                return QVariant(QString::fromStdString(contact.registeredName));
            else
                return QVariant(QString::fromStdString(contact.uri));
        }
        case Role::Presence:
        {
            // TODO Handle conversations with multiple contacts
            auto& contact = acc_.contactModel->getContact(item->participants[0]);
            return QVariant(contact.isPresent);
        }
        case Role::URI:
        {
            // TODO Handle conversations with multiple contacts
            auto& contact = acc_.contactModel->getContact(item->participants[0]);
            return QVariant(QString::fromStdString(contact.uri));
        }
        case Role::UnreadMessagesCount:
            return QVariant(item->unreadMessages);
        case Role::LastInteractionDate:
        {
            auto& date = item->messages.at(item->lastMessageUid).timestamp;
            return QVariant(QDateTime::fromTime_t(date));
        }
        case Role::LastInteraction:
            return QVariant(QString());
        }
    }

    return QVariant();
}

QModelIndex SmartListModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    if (column != 0)
        return QModelIndex();

    if (row >= 0 && row < rowCount())
        return createIndex(row, column);

    return QModelIndex();
}

QModelIndex SmartListModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)

    return QModelIndex();
}

Qt::ItemFlags SmartListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);
    else
        return QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren;
}
