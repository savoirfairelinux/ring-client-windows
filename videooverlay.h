/***************************************************************************
 * Copyright (C) 2015-2019 by Savoir-faire Linux                           *
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>*
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

#include "api/conversationmodel.h"

#include <QWidget>
#include <QMenu>
#include <QTimer>
#include <QPushButton>
#include <QEvent>

class ContactPicker;
class VideoView;
namespace Ui {
class VideoOverlay;
}

class VideoOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit VideoOverlay(QWidget* parent = 0);
    ~VideoOverlay();

public:
    void setName(const QString& name);
    void callStarted(const std::string & callId);
    inline bool isDialogVisible(){ return dialogVisible_; };
    void toggleContextButtons(bool visible);
    void setVideoMuteVisibility(bool visible);
    bool shouldShowOverlay();
    void simulateShowChatview(bool checked);
    bool getShowChatView();
    void setTransferCallAvailability(bool visible);
    void setCurrentSelectedCalleeDisplayName(const QString& CalleeDisplayName);
    void resetOverlay(bool isAudioMuted, bool isVideoMuted, bool isRecording, bool isHolding, bool isAudioOnly);

//UI SLOTS
private slots:
    void setTime();
    void on_hangupButton_clicked();
    void on_chatButton_toggled(bool checked);
    void on_holdButton_toggled(bool checked);
    void on_noMicButton_toggled(bool checked);
    void on_noVideoButton_toggled(bool checked);
    void on_recButton_clicked();
    void on_transferCallButton_toggled(bool checked);
    void slotWillDoTransfer(const std::string& callId, const std::string& contactUri);

private:
    Ui::VideoOverlay* ui;
    ContactPicker* contactPicker_;
    bool dialogVisible_ = false;
    QTimer* oneSecondTimer_;
    std::string callId_;
    std::map<QPushButton*, QPixmap> originalIconImageMap_;
    std::map<QPushButton*, QPixmap> originalCheckedIconImageMap_;
    std::map<QPushButton*, QPixmap> tintIconImageMap_;
    std::map<QPushButton*, QPixmap> tintCheckedIconImageMap_;

private:
    bool eventFilter(QObject* target, QEvent* event);
    void initializeBtnEventListener();
    bool setButtonsIconForEvent(QObject* target, QEvent* event);
    void initializeIconImageContainers();

signals:
    void setChatVisibility(bool visible);
    void HoldStatusChanged(bool pauseLabelStatus);

};
