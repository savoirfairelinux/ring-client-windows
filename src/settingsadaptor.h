/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>

#include "api/account.h"
#include "api/datatransfermodel.h"
#include "lrcinstance.h"
#include "typedefs.h"
#include "utils.h"

class SettingsAdaptor : public QObject
{
    Q_OBJECT
public:
    explicit SettingsAdaptor(QObject *parent = nullptr);

    const Q_INVOKABLE lrc::api::account::Info &getCurrentAccountInfo();

signals:
};
