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
#include <QSettings>

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

    // getters of directories
    Q_INVOKABLE QString getDir_Document();
    Q_INVOKABLE QString getDir_Download();

    // getters and setters of app settings options
    Q_INVOKABLE bool getSettingsValue_CloseOrMinimized();
    Q_INVOKABLE bool getSettingsValue_EnableNotifications();
    Q_INVOKABLE bool getSettingsValue_AutoUpdate();

    Q_INVOKABLE void setClosedOrMin(bool state);
    Q_INVOKABLE void setNotifications(bool state);
    Q_INVOKABLE void setUpdateAutomatic(bool state);
    Q_INVOKABLE void setRunOnStartUp(bool state);
    Q_INVOKABLE void setDownloadPath(QString dir);

    // getters of devices' Info and options
    Q_INVOKABLE lrc::api::video::Capabilities get_DeviceCapabilities(const QString &device);
    Q_INVOKABLE lrc::api::video::ResRateList get_ResRateList(lrc::api::video::Channel channel,
                                                             QString device);
    Q_INVOKABLE int get_DeviceCapabilitiesSize(const QString &device);

    // getters of resolution and frame rates of current device
    Q_INVOKABLE QVector<QString> getResolutions(const QString &device);
    Q_INVOKABLE QVector<int> getFrameRates(const QString &device);

    // getters and setters: lrc video::setting
    Q_INVOKABLE QString get_Video_Settings_Channel(const QString &deviceId);
    Q_INVOKABLE QString get_Video_Settings_Name(const QString &deviceId);
    Q_INVOKABLE QString get_Video_Settings_Id(const QString &deviceId);
    Q_INVOKABLE qreal get_Video_Settings_Rate(const QString &deviceId);
    Q_INVOKABLE QString get_Video_Settings_Size(const QString &deviceId);

    Q_INVOKABLE void set_Video_Settings_Rate_And_Resolution(const QString &deviceId,
                                                            qreal rate,
                                                            const QString &resolution);

    // getters and setters of current account Info
    const Q_INVOKABLE lrc::api::account::Info &getCurrentAccountInfo();
    Q_INVOKABLE ContactModel *getContactModel();
    Q_INVOKABLE NewDeviceModel *getDeviceModel();
};
