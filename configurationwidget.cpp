/***************************************************************************
 * Copyright (C) 2015-2017 by Savoir-faire Linux                           *
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>*
 * Author: Anthony Léonard <anthony.leonard@savoirfairelinux.com>          *
 * Author: Olivier Soldano <olivier.soldano@savoirfairelinux.com>          *
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

#include "configurationwidget.h"
#include "ui_configurationwidget.h"

#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QFileDialog>
#include <QPropertyAnimation>
#include <QtConcurrent/QtConcurrent>

#include "deleteaccountdialog.h"
#include "utils.h"
#include "photoboothdialog.h"
#include "wizarddialog.h"
#include "mainwindow.h"

// LRC
#include "video/devicemodel.h"
#include "video/channel.h"
#include "video/resolution.h"
#include "video/rate.h"
#include "video/previewmanager.h"

#include "audio/settings.h"
#include "audio/outputdevicemodel.h"
#include "audio/inputdevicemodel.h"

#include "media/recordingmodel.h"

#include "accountserializationadapter.h"
#include "accountstatedelegate.h"
#include "settingskey.h"

#include "accountmodel.h"
#include "protocolmodel.h"
#include "accountdetails.h"
#include "callmodel.h"
#include "ringtonemodel.h"
#include "categorizedhistorymodel.h"
#include "profilemodel.h"
#include "profile.h"
#include "person.h"

#include "api/lrc.h"

#include "winsparkle.h"

ConfigurationWidget::ConfigurationWidget(QWidget *parent) :
    NavWidget(parent),
    ui(new Ui::ConfigurationWidget),
    accountDetails_(new AccountDetails())
{
    initUI();
}

ConfigurationWidget::~ConfigurationWidget()
{
    delete ui;
}

void
ConfigurationWidget::initUI()
{
    ui->setupUi(this);

    connect(ui->exitSettingsButton, &QPushButton::clicked, this, [=]() {
        emit NavigationRequested(ScreenEnum::CallScreen);
    });

    ui->accountView->setItemDelegate(new AccountStateDelegate());

    // connect delete button to popup trigger
    connect(ui->deleteAccountBtn, &QPushButton::clicked, [=](){
        auto idx = ui->accountView->currentIndex();
        DeleteAccountDialog dialog(idx);
        dialog.exec();
    });

    isLoading_ = true;

    ui->videoView->setIsFullPreview(true);

    connect(ui->generalTabButton, &QPushButton::toggled, [=] (bool toggled) {
        if (toggled) {
            Utils::slidePage(ui->stackedWidget, ui->generalPage);
            ui->videoTabButton->setChecked(false);
            ui->accountTabButton->setChecked(false);
        }
    });

    connect(ui->videoTabButton, &QPushButton::toggled, [=] (bool toggled) {
        if (toggled) {
            Utils::slidePage(ui->stackedWidget, ui->videoPage);
            ui->accountTabButton->setChecked(false);
            ui->generalTabButton->setChecked(false);
        }
    });

    connect(ui->accountTabButton, &QPushButton::toggled, [=] (bool toggled) {
        if (toggled) {
            Utils::slidePage(ui->stackedWidget, ui->accountPage);
            ui->videoTabButton->setChecked(false);
            ui->generalTabButton->setChecked(false);
        }
    });

    ui->generalTabButton->setChecked(true);

    //temporary fix hiding imports buttons
    ui->exportButton->hide();

    ui->intervalUpdateCheckSpinBox->setEnabled(true);

    // doesnt work with new syntax
    connect(ui->outputComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(outputIndexChanged(int)));
    connect(ui->inputComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(inputIndexChanged(int)));
}

void
ConfigurationWidget::initLrcConnections()
{
    // models used
    accountModel_ = new ClientAccountModel(lrc_->getAccountModel());
    deviceModel_ = &Video::DeviceModel::instance();

    connect(ui->exitSettingsButton, &QPushButton::clicked, this, [=]() {
        if (CallModel::instance().getActiveCalls().size() == 0
                && Video::PreviewManager::instance().isPreviewing()) {
            Video::PreviewManager::instance().stopPreview();
        }
        AccountModel::instance().save();
        accountDetails_->save();
    });

    ui->accountView->setModel(accountModel_);
    ui->deviceBox->setModel(deviceModel_);
    connect(deviceModel_, &Video::DeviceModel::currentIndexChanged, this, &ConfigurationWidget::deviceIndexChanged);

    if (ui->deviceBox->count() > 0){
        ui->deviceBox->setCurrentIndex(0);
    }

    // accounts
    AccountModel::instance().selectionModel()->clear();
    ui->accountView->setSelectionModel(AccountModel::instance().selectionModel());
    connect(ui->accountView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(accountSelected(QItemSelection)));

    ui->accountView->setCurrentIndex(accountModel_->index(0));
    ui->accountDetailLayout->addWidget(accountDetails_);
    ui->accountTypeBox->setModel(AccountModel::instance().protocolModel());
    ui->accountTypeBox->setCurrentIndex(ui->accountTypeBox->findText("RING"));
    ui->startupBox->setChecked(Utils::CheckStartupLink());

    ui->historyDaySettingsSpinBox->setValue(
                CategorizedHistoryModel::instance().historyLimit());
    ui->closeOrMinCheckBox->setChecked(settings_.value(
                                           SettingsKey::closeOrMinimized).toBool());
    ui->notificationCheckBox->setChecked(settings_.value(
                                           SettingsKey::enableNotifications).toBool());
    connect(ui->stackedWidget, &QStackedWidget::currentChanged, [](int index) {
        if (index == 1
                && CallModel::instance().getActiveCalls().size() == 0) {
            Video::PreviewManager::instance().startPreview();
        } else {
            if (CallModel::instance().getActiveCalls().size() == 0
                    && Video::PreviewManager::instance().isPreviewing()) {
                Video::PreviewManager::instance().stopPreview();
            }
        }
    });

    auto recordPath = Media::RecordingModel::instance().recordPath();
    if (recordPath.isEmpty()) {
        recordPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        Media::RecordingModel::instance().setRecordPath(recordPath);
    }
    ui->recordPath->setText(Media::RecordingModel::instance().recordPath());

    ui->alwaysRecordCheckBox->setChecked(Media::RecordingModel::instance().isAlwaysRecording());
    connect(ui->alwaysRecordCheckBox, &QCheckBox::clicked, [](bool checked){
        Media::RecordingModel::instance().setAlwaysRecording(checked);
    });


    // Audio settings
    auto inputModel = Audio::Settings::instance().inputDeviceModel();
    auto outputModel = Audio::Settings::instance().outputDeviceModel();

    ui->outputComboBox->setModel(outputModel);
    ui->inputComboBox->setModel(inputModel);
    if(ui->outputComboBox->count() > 0) {
        ui->outputComboBox->setCurrentIndex(0);
    }
    if (ui->inputComboBox->count() > 0){
        ui->inputComboBox->setCurrentIndex(0);
    }

    // profile
    auto& currentAccountIndex = accountModel_->selectedAccountIndex();
    ui->avatarButton->setIcon(QPixmap::fromImage(
                                  Utils::getCirclePhoto(currentAccountIndex.data(ClientAccountModel::Role::Avatar).value<QImage>(),
                                                        ui->avatarButton->width())));
    ui->profileNameEdit->setText(currentAccountIndex.data(ClientAccountModel::Role::Alias).value<QString>());
}

void
ConfigurationWidget::setLrc(std::shared_ptr<lrc::api::Lrc>& lrc)
{
    lrc_ = lrc;
}

void
ConfigurationWidget::showPreview()
{
    if (ui->stackedWidget->currentIndex() == 1
            && CallModel::instance().getActiveCalls().size() == 0) {
        ui->previewUnavailable->hide();
        ui->videoView->show();
        Video::PreviewManager::instance().startPreview();
    } else {
        ui->previewUnavailable->show();
        ui->videoView->hide();
    }
}

void
ConfigurationWidget::showEvent(QShowEvent *event)
{
    if (win_sparkle_get_automatic_check_for_updates()) {
        ui->autoUpdateCheckBox->setChecked(true);
    }
    ui->intervalUpdateCheckSpinBox->setValue(win_sparkle_get_update_check_interval() / 86400);
    QWidget::showEvent(event);
    showPreview();
}


void
ConfigurationWidget::deviceIndexChanged(int index)
{
    ui->deviceBox->setCurrentIndex(index);
}

void
ConfigurationWidget::on_deviceBox_currentIndexChanged(int index)
{
    if (index < 0)
        return;

    if (!isLoading_)
        deviceModel_->setActive(index);

    auto device = deviceModel_->activeDevice();

    ui->sizeBox->clear();

    isLoading_ = true;
    if (device->channelList().size() > 0) {
        for (auto resolution : device->channelList()[0]->validResolutions()) {
            ui->sizeBox->addItem(resolution->name());
        }
    }
    ui->sizeBox->setCurrentIndex(
                device->channelList()[0]->activeResolution()->relativeIndex());
    isLoading_ = false;
}

void
ConfigurationWidget::on_sizeBox_currentIndexChanged(int index)
{
    auto device = deviceModel_->activeDevice();

    if (index < 0)
        return;
    if (!isLoading_)
        device->channelList()[0]->setActiveResolution(
                    device->channelList()[0]->validResolutions()[index]);
}

void
ConfigurationWidget::accountSelected(QItemSelection itemSel)
{
    if (itemSel.size())
        accountDetails_->show();
    else
        accountDetails_->hide();

    if (accountConnection_)
        disconnect(accountConnection_);

    auto account = AccountModel::instance().getAccountByModelIndex(
                ui->accountView->currentIndex());
    accountDetails_->setAccount(account);
    if (account) {
        AccountSerializationAdapter adapter(account, accountDetails_, accountModel_);
        accountConnection_= connect(account,
                                    SIGNAL(propertyChanged(Account*,QString,QString,QString)),
                                    this,
                                    SLOT(accountPropertyChanged(Account*,QString,QString,QString)));
    }
}

void
ConfigurationWidget::accountPropertyChanged(Account* a,
                                            const QString& name,
                                            const QString& newVal,
                                            const QString& oldVal)
{
    Q_UNUSED(name)
    Q_UNUSED(newVal)
    Q_UNUSED(oldVal)
    accountDetails_->setAccount(a);
    AccountSerializationAdapter adapter(a, accountDetails_, accountModel_);
}

void
ConfigurationWidget::on_addAccountButton_clicked()
{
    auto type = ui->accountTypeBox->model()->index(ui->accountTypeBox->currentIndex(), 0);
    if (type.data()  == "RING") {
        WizardDialog dlg(WizardDialog::NEW_ACCOUNT);
        dlg.exec();
    } else {
        auto account = AccountModel::instance().add(tr("New Account"), type);
        account->setRingtonePath(Utils::GetRingtonePath());
        AccountModel::instance().save();
    }
}

void
ConfigurationWidget::on_startupBox_toggled(bool checked)
{
    if (checked)
        Utils::CreateStartupLink();
    else
        Utils::DeleteStartupLink();
}

void
ConfigurationWidget::on_clearHistoryButton_clicked()
{
    QMessageBox confirmationDialog;

    confirmationDialog.setText(tr("Are you sure you want to clear all your history?"));
    confirmationDialog.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    auto ret = confirmationDialog.exec();

    if (ret == QMessageBox::Ok)
        CategorizedHistoryModel::instance().clearAllCollections();
}

void
ConfigurationWidget::on_historyDaySettingsSpinBox_valueChanged(int limit)
{
    if (CategorizedHistoryModel::instance().historyLimit() != limit)
        CategorizedHistoryModel::instance().setHistoryLimit(limit);
}

void
ConfigurationWidget::on_closeOrMinCheckBox_toggled(bool checked)
{
    settings_.setValue(SettingsKey::closeOrMinimized, checked);
}

void
ConfigurationWidget::on_checkUpdateButton_clicked()
{
    win_sparkle_check_update_with_ui();
}

void
ConfigurationWidget::on_autoUpdateCheckBox_toggled(bool checked)
{
    win_sparkle_set_automatic_check_for_updates(checked);
}

void
ConfigurationWidget::on_intervalUpdateCheckSpinBox_valueChanged(int arg1)
{
    win_sparkle_set_update_check_interval(arg1 * 86400);
}

void
ConfigurationWidget::on_stackedWidget_currentChanged(int index)
{
    Q_UNUSED(index)
    showPreview();
}

void
ConfigurationWidget::on_recordPath_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Directory"),
                                                 Media::RecordingModel::instance().recordPath(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if (not dir.isEmpty()) {
        Media::RecordingModel::instance().setRecordPath(dir);
        ui->recordPath->setText(dir);
    }
}

void
ConfigurationWidget::outputIndexChanged(int index)
{
    auto outputModel = Audio::Settings::instance().outputDeviceModel();
    outputModel->selectionModel()->setCurrentIndex(outputModel->index(index), QItemSelectionModel::ClearAndSelect);
}

void
ConfigurationWidget::inputIndexChanged(int index)
{
    auto inputModel = Audio::Settings::instance().inputDeviceModel();
    inputModel->selectionModel()->setCurrentIndex(inputModel->index(index), QItemSelectionModel::ClearAndSelect);
}

void
ConfigurationWidget::on_exportButton_clicked()
{
   /*
    PathPasswordDialog dlg(true);
    if (dlg.exec() == QDialog::Accepted) {
        auto func = [](QString path, QString password)
        {
            AccountModel::instance().exportAccounts(
            {AccountModel::instance().selectedAccount()->id()},
                        path,
                        password);
        };
        QtConcurrent::run(func, dlg.path_, dlg.password_);
    }
    */
}


void
ConfigurationWidget::on_avatarButton_clicked()
{
    PhotoBoothDialog dlg;
    dlg.exec();
    if (dlg.result() == QDialog::Accepted) {
        auto image = QImage(dlg.getOutputFileName());
        auto avatar = image.scaled(100, 100, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        ProfileModel::instance().selectedProfile()->person()->setPhoto(avatar);
        ProfileModel::instance().selectedProfile()->save();
        ui->avatarButton->setIcon(QPixmap::fromImage(Utils::getCirclePhoto(avatar, ui->avatarButton->width())));
    }
}

void
ConfigurationWidget::on_profileNameEdit_textEdited(const QString& name)
{
    ProfileModel::instance().selectedProfile()->person()->setFormattedName(name);
    ProfileModel::instance().selectedProfile()->save();
}

void
ConfigurationWidget::on_notificationCheckBox_toggled(bool checked)
{
    settings_.setValue(SettingsKey::enableNotifications, checked);
}
