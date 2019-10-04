/**************************************************************************
| Copyright (C) 2019 by Savoir-faire Linux                                |
| Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>          |
| Author: Isa Nanic <isa.nanic@savoirfairelinux.com>                      |
| Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>              |
|                                                                         |
| This program is free software; you can redistribute it and/or modify    |
| it under the terms of the GNU General Public License as published by    |
| the Free Software Foundation; either version 3 of the License, or       |
| (at your option) any later version.                                     |
|                                                                         |
| This program is distributed in the hope that it will be useful,         |
| but WITHOUT ANY WARRANTY; without even the implied warranty of          |
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           |
| GNU General Public License for more details.                            |
|                                                                         |
| You should have received a copy of the GNU General Public License       |
| along with this program.  If not, see <https://www.gnu.org/licenses/>.  |
**************************************************************************/
#pragma once

#ifdef _MSC_VER
#undef ERROR
#endif

#include <QObject>
#include <QMutex>
#include <QSettings>
#include <QRegularExpression>
#include <QPixmap>
#include <QBuffer>

#include "settingskey.h"
#include "accountlistmodel.h"
#include "utils.h"
#include "rendermanager.h"

#include "api/lrc.h"
#include "api/account.h"
#include "api/newaccountmodel.h"
#include "api/newcallmodel.h"
#include "api/newdevicemodel.h"
#include "api/newcodecmodel.h"
#include "api/behaviorcontroller.h"
#include "api/avmodel.h"
#include "api/conversation.h"
#include "api/contactmodel.h"
#include "api/contact.h"
#include "api/datatransfermodel.h"
#include "api/conversationmodel.h"
#include "api/peerdiscoverymodel.h"

#include <memory>

using namespace lrc::api;

using migrateCallback = std::function<void()>;

class LRCInstance : public QObject
{
    Q_OBJECT

public:
    static LRCInstance& instance(migrateCallback willMigrate = {},
                                 migrateCallback didMigrate = {}) {
        static LRCInstance instance_(willMigrate, didMigrate);
        return instance_;
    };
    static void init(migrateCallback willMigrate = {},
                     migrateCallback didMigrate = {}) {
        instance(willMigrate, didMigrate);
    };
    static Lrc& getAPI() {
        return *(instance().lrc_);
    };
    static RenderManager* renderer() {
        return instance().renderer_.get();
    }
    static void connectivityChanged() {
        instance().lrc_->connectivityChanged();
    };
    static NewAccountModel& accountModel() {
        return instance().lrc_->getAccountModel();
    };
    static BehaviorController& behaviorController() {
        return instance().lrc_->getBehaviorController();
    };
    static DataTransferModel& dataTransferModel() {
        return instance().lrc_->getDataTransferModel();
    };
    static AVModel& avModel() {
        return instance().lrc_->getAVModel();
    };
    static bool isConnected() {
        return instance().lrc_->isConnected();
    };
    static std::vector<std::string> getActiveCalls() {
        return instance().lrc_->activeCalls();
    };
    static const account::Info&
    getCurrentAccountInfo() {
        try {
            return accountModel().getAccountInfo(getCurrAccId());
        } catch (...) {
            static account::Info invalid = {};
            qWarning() << "getAccountInfo exception";
            return invalid;
        }
    };
    static bool hasVideoCall() {
        auto activeCalls = instance().lrc_->activeCalls();
        auto accountList = accountModel().getAccountList();
        bool result = false;
        for (const auto& callId : activeCalls) {
            for (const auto& accountId : accountList) {
                auto& accountInfo = accountModel().getAccountInfo(accountId);
                if (accountInfo.callModel->hasCall(callId)) {
                    result |= !accountInfo.callModel->getCall(callId).isAudioOnly;
                }
            }
        }
        return result;
    };
    static const call::Info*
    getCallInfoForConversation(const conversation::Info& convInfo) {
        auto& accInfo = LRCInstance::accountModel().getAccountInfo(convInfo.accountId);
        if (!accInfo.callModel->hasCall(convInfo.callId)) {
            return nullptr;
        }
        return &accInfo.callModel->getCall(convInfo.callId);
    }

    static ConversationModel*
    getCurrentConversationModel() {
        return getCurrentAccountInfo().conversationModel.get();
    };

    static NewCallModel*
    getCurrentCallModel() {
        return getCurrentAccountInfo().callModel.get();
    };

    static const std::string& getCurrAccId() {
        auto accountList = accountModel().getAccountList();
        if (instance().selectedAccountId_.empty() && accountList.size()) {
            instance().selectedAccountId_ = accountList.at(0);
        }
        return instance().selectedAccountId_;
    };

    static void setSelectedAccountId(const std::string& accountId = {}) {
        instance().selectedAccountId_ = accountId;
        QSettings settings("jami.net", "Jami");
        settings.setValue(SettingsKey::selectedAccount, QString::fromStdString(accountId));
    };

    static const std::string& getSelectedConvUid() {
        return instance().selectedConvUid_;
    };

    static void setSelectedConvId(const std::string& convUid = {}) {
        instance().selectedConvUid_ = convUid;
    };

    static void reset(bool newInstance = false) {
        if (newInstance) {
            instance().renderer_.reset(new RenderManager(avModel()));
            instance().lrc_.reset(new Lrc());
        } else {
            instance().renderer_.reset();
            instance().lrc_.reset();
        }
    };

    static const int getCurrentAccountIndex(){
        for (int i = 0; i < accountModel().getAccountList().size(); i++) {
            if (accountModel().getAccountList()[i] == getCurrAccId()) {
                return i;
            }
        }
        return -1;
    };

    static const QPixmap getCurrAccPixmap() {
        return instance().accountListModel_.data(instance().accountListModel_
            .index(getCurrentAccountIndex()), AccountListModel::Role::Picture).value<QPixmap>();
    };

    static void setCurrAccAvatar(const QPixmap& avatarPixmap) {
        QByteArray ba;
        QBuffer bu(&ba);
        bu.open(QIODevice::WriteOnly);
        avatarPixmap.save(&bu, "PNG");
        auto str = ba.toBase64().toStdString();
        accountModel().setAvatar(getCurrAccId(), str);
    };

    static void setCurrAccAvatar(const std::string& avatar) {
        accountModel().setAvatar(getCurrAccId(), avatar);
    };

    static void setCurrAccDisplayName(const std::string& alias) {
        accountModel().setAlias(getCurrAccId(), alias);
    };

    static const account::ConfProperties_t& getCurrAccConfig() {
        return instance().getCurrentAccountInfo().confProperties;
    }

    static void subscribeToDebugReceived() {
        instance().lrc_->subscribeToDebugReceived();
    }

signals:
    void accountListChanged();

private:
    std::unique_ptr<Lrc> lrc_;
    AccountListModel accountListModel_;

    LRCInstance(migrateCallback willMigrateCb = {},
                migrateCallback didMigrateCb = {}) {
        lrc_ = std::make_unique<Lrc>(willMigrateCb, didMigrateCb);
        renderer_ = std::make_unique<RenderManager>(lrc_->getAVModel());
    };

    std::string selectedAccountId_;
    std::string selectedConvUid_;

    std::unique_ptr<RenderManager> renderer_;
};
