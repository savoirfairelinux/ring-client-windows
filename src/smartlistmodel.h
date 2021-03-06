/**************************************************************************
* Copyright (C) 2017-2019 by Savoir-faire Linux                           *
* Author: Anthony Léonard <anthony.leonard@savoirfairelinux.com>          *
* Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>          *
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
#pragma once

#include "api/account.h"
#include "api/conversation.h"
#include "api/conversationmodel.h"
#include "api/contact.h"

#include <QAbstractItemModel>

using namespace lrc::api;

class SmartListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    using AccountInfo = lrc::api::account::Info;
    using ConversationInfo = lrc::api::conversation::Info;
    using ContactInfo = lrc::api::contact::Info;

    enum class Type {
        CONVERSATION,
        CONFERENCE,
        TRANSFER,
        COUNT__
    };

    enum Role {
        DisplayName = Qt::UserRole + 1,
        DisplayID,
        Picture,
        Presence,
        URI,
        UnreadMessagesCount,
        LastInteractionDate,
        LastInteraction,
        LastInteractionType,
        ContactType,
        UID,
        ContextMenuOpen,
        InCall,
        CallStateStr,
        SectionName,
        AccountId,
        Draft
    };

    explicit SmartListModel(const QString& accId,
                            QObject *parent = 0,
                            SmartListModel::Type listModelType = Type::CONVERSATION,
                            const QString& convUid = {});

    // QAbstractItemModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void setAccount(const QString& accId);
    void setConferenceableFilter(const QString& filter = {});
    void toggleSection(const QString& section);

    // hack for context menu highlight retention
    bool isContextMenuOpen{ false };

private:
    QString accountId_;

    QVariant getConversationItemData(const ConversationInfo& item,
                                     const AccountInfo& accountInfo,
                                     int role) const;
    // list sectioning
    QString convUid_;
    Type listModelType_;
    QMap<QString, bool> sectionState_;
    QMap<ConferenceableItem, ConferenceableValue> conferenceables_;

};