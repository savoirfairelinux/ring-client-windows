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
#ifndef CLIENTACCOUNTMODEL_H
#define CLIENTACCOUNTMODEL_H

#include <QAbstractItemModel>
#include <QPersistentModelIndex>

// LRC
#include "api/account.h"
#include "api/profile.h"

Q_DECLARE_METATYPE(lrc::api::profile::Type)
Q_DECLARE_METATYPE(lrc::api::account::Status)

namespace lrc { namespace api { class NewAccountModel; } }

class ClientAccountModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Role {
        Alias = Qt::UserRole+1,
        SmartListModel,
        Avatar,
//        Id,
        Uri,
        BestId,
        BestName,
        Type
    };

    explicit ClientAccountModel(const lrc::api::NewAccountModel &mdl, QObject *parent = 0);

    // QAbstractItemModel
    QModelIndex index(int row, int column = 0,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    // newaccountmodel wrappers
    std::vector<std::string> getAccountList() const;
    const lrc::api::account::Info &getAccountInfo(const std::string& accountId) const;

    // client side actions
    QModelIndex selectedAccountIndex();

public slots:
    void setSelectedAccount(QModelIndex& newAccountIndex);

signals:
    void currentAccountChanged(const QModelIndex& newIndex);

private:
    const lrc::api::NewAccountModel& mdl_;
    int selectedAccountRow_;
};

#endif // CLIENTACCOUNTMODEL_H
