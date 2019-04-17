/***************************************************************************
 * Copyright (C) 2019-2019 by Savoir-faire Linux                           *
 * Author: Isa Nanic <isa.nanic@savoirfairelinux.com>
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.   *
 **************************************************************************/

#pragma once
#include <QMovie>
#include <QScrollArea>

#include "lrcinstance.h"
#include "navwidget.h"

#include "advancedsettingswidget.h"
#include "advancedsipsettingwidget.h"
#include "bannedlistmodel.h"

#include "linkdevwidget.h"
#include "ui_linkdevwidget.h"

// general settings
#include "api/datatransfermodel.h"
#include "typedefs.h"

// av settings
#include "video/devicemodel.h"

namespace Ui {
class SettingsWidget;
}

class SettingsWidget : public NavWidget {
    Q_OBJECT
    SettingsWidget(const SettingsWidget& cpy);

public:
    explicit SettingsWidget(QWidget* parent = nullptr);
    ~SettingsWidget();

    void resize(int size);

    // NavWidget
    virtual void navigated(bool to);
    virtual void updateCustomUI();
public slots:
    virtual void slotAccountOnBoarded();

private:
    Ui::SettingsWidget* ui;

    enum Button { accountSettingsButton,
        generalSettingsButton,
        avSettingsButton };
    enum RegName { BLANK,
        INVALIDFORM,
        TAKEN,
        FREE,
        SEARCHING };
    enum List { DevList,
        BannedContacts };

    void populateGeneralSettings();
    void populateAVSettings();
    void setFormatListForDevice(Video::Device* device);
    void showPreview();
    void startVideo();
    void stopVideo();
    void toggleVideoSettings(bool enabled);
    void toggleVideoPreview(bool enabled);
    bool showOrHide_{false};
    void showhideButtonClicked();
    void passwordClicked();
    void avatarClicked();
    void afterNameLookup(lrc::api::account::LookupStatus status, const std::string& regName);
    bool validateRegNameForm(const QString& regName);
    void setRegNameUi(RegName stat);
    void removeDeviceSlot(int index);
    void unban(int index);
    void setConnections();
    void setSelected(Button sel);
    void updateAccountInfoDisplayed();
    void resizeEvent(QResizeEvent* event);

    QList<QPair<int, int>> formatIndexList_;
    Video::DeviceModel* deviceModel_;
    QString currentDeviceName_;
    lrc::api::account::ConfProperties_t confProps_;
    QMovie* gif;
    QString registeredName_;
    AdvancedSettingsWidget* advancedSettingsWidget_;
    AdvancedSIPSettingsWidget* advancedSIPSettingsWidget_;
    QScrollArea* scrollArea_;
    QScrollArea* scrollSIPArea_;
    Button pastButton_ = Button::generalSettingsButton;
    bool advancedSettingsDropped_ = false;
    bool bannedContactsShown_ = false;
    bool advancedSIPSettingsDropped_ = false;
    int avatarSize_;
    int avatarSIPSize_;
    bool regNameBtn_ = false;
    const int itemHeight_ = 55;
    LinkDevWidget* linkDevWidget;

private slots:
    void leaveSettingsSlot();
    void verifyRegisteredNameSlot();
    void beforeNameLookup();
    void receiveRegNameSlot(const std::string& accountID, lrc::api::account::LookupStatus status,
        const std::string& address, const std::string& name);
    void regNameRegisteredSlot();
    void setAccEnableSlot(int state);
    void delAccountSlot();
    void toggleAdvancedSIPSettings();
    void toggleAdvancedSettings();
    void toggleBannedContacts();
    void exportAccountSlot();
    void updateAndShowDevicesSlot();
    void updateAndShowBannedContactsSlot();
    void showLinkDevSlot();
    void showCurrentAccountSlot();
    void setButtonIconSlot(int frame);
    void setNotificationsSlot(int state);
    void checkForUpdateSlot();
    void setClosedOrMinSlot(int state);
    void openDownloadFolderSlot();
    void setAlwaysRecordingSlot(int state);
    void openRecordFolderSlot();
    void setUpdateIntervalSlot(int value);
    void setUpdateAutomaticSlot(int state);
    void outputDevIndexChangedSlot(int index);
    void inputdevIndexChangedSlot(int index);
    void deviceModelIndexChanged(int index);
    void slotDeviceBoxCurrentIndexChanged(int index);
    void slotFormatBoxCurrentIndexChanged(int index);
    void updateSettings(int size);
};
