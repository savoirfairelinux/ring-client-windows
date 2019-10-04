/***************************************************************************
 * Copyright (C) 2015-2019 by Savoir-faire Linux                           *
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>*
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

#include "videoview.h"
#include "ui_videoview.h"

#include "lrcinstance.h"
#include "utils.h"

#include <QDesktopWidget>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QMenu>
#include <QMimeData>
#include <QPropertyAnimation>
#include <QScreen>
#include <QSplitter>
#include <QWindow>

#include <memory>

#include "selectareadialog.h"
#include "videooverlay.h"

VideoView::VideoView(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::VideoView)
{
    ui->setupUi(this);

    // video overlay
    overlay_ = new VideoOverlay(this);
    overlay_->setMouseTracking(true);

    // context menu
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showContextMenu(const QPoint&)));

    // chat panel
    connect(overlay_, &VideoOverlay::setChatVisibility,
        [=](bool visible) {
            emit this->setChatVisibility(visible);
            // what is this?
            connect(this, SIGNAL(toggleFullScreenClicked()),
                    ui->videoWidget, SLOT(slotToggleFullScreenClicked()));
        });

    // audio only overlay
    audioOnlyAvatar_ = new CallAudioOnlyAvatarOverlay(this);

    // preview widget
    previewWidget_ = new VideoCallPreviewWidget(this);

    // preview widget animation
    moveAnim_ = new QPropertyAnimation(previewWidget_, "geometry");
    moveAnim_->setDuration(200);
    moveAnim_->setEasingCurve(QEasingCurve::OutQuad);
}

VideoView::~VideoView()
{
    closing(pConvInfo_->callId);
    delete ui;
    delete overlay_;

}

void
VideoView::resizeEvent(QResizeEvent* event)
{
    moveAnim_->stop();

    previewWidget_->setContainerSize(event->size());
    resetPreviewPosition();
    previewWidget_->forceRepaint();

    audioOnlyAvatar_->resize(this->size());

    overlay_->resize(this->size());
    overlay_->show();
    overlay_->raise();
}

void
VideoView::slotCallStatusChanged(const std::string& callId)
{
    if (callId != pConvInfo_->callId) {
        return;
    }

    using namespace lrc::api::call;
    auto& accInfo = LRCInstance::accountModel().getAccountInfo(pConvInfo_->accountId);
    auto call = accInfo.callModel->getCall(pConvInfo_->callId);
    switch (call.status) {
    case Status::ENDED:
    case Status::TERMINATING:
        emit closing(pConvInfo_->callId);
    default:
        break;
    }
}

void
VideoView::simulateShowChatview(bool checked)
{
    Q_UNUSED(checked);
    overlay_->simulateShowChatview(true);
}

void
VideoView::mouseDoubleClickEvent(QMouseEvent* e)
{
    QWidget::mouseDoubleClickEvent(e);
    toggleFullScreen();
}

void
VideoView::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
        event->accept();
}

void
VideoView::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();
}
void
VideoView::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasUrls())
        event->accept();
}

void
VideoView::dropEvent(QDropEvent* event)
{
    // take only the first file
    QString urlString = event->mimeData()->urls().at(0).toString();
    auto selectedConvUid = LRCInstance::getSelectedConvUid();
    auto convModel = LRCInstance::getCurrentConversationModel();
    auto conversation = Utils::getConversationFromUid(selectedConvUid, *convModel);
    auto callIdList = LRCInstance::getActiveCalls();
    for (auto callId : callIdList) {
        if (callId == conversation->callId) {
            LRCInstance::avModel().setInputFile(urlString.toStdString());
            break;
        }
    }
}

void
VideoView::toggleFullScreen()
{
    emit toggleFullScreenClicked();
}

void
VideoView::showContextMenu(const QPoint& pos)
{
    QPoint globalPos = this->mapToGlobal(pos);

    QMenu menu;

    auto selectedConvUid = LRCInstance::getSelectedConvUid();
    auto convModel = LRCInstance::getCurrentConversationModel();
    auto conversation = Utils::getConversationFromUid(selectedConvUid, *convModel);
    auto callIdList = LRCInstance::getActiveCalls();
    std::string thisCallId;
    for (auto callId : callIdList) {
        if (callId == conversation->callId) {
            thisCallId = callId;
            break;
        }
    }
    if (thisCallId.empty()) {
        return;
    }

    auto activeDevice = LRCInstance::avModel().getCurrentRenderedDevice(thisCallId);

    // video input devices
    auto devices = LRCInstance::avModel().getDevices();
    auto device = LRCInstance::avModel().getDefaultDeviceName();
    for (auto d : devices) {
        auto deviceName = QString::fromStdString(d).toUtf8();
        auto deviceAction = new QAction(deviceName, this);
        menu.addAction(deviceAction);
        deviceAction->setCheckable(true);
        if (d == activeDevice.name) {
            deviceAction->setChecked(true);
        }
        connect(deviceAction, &QAction::triggered,
            [this, deviceName, thisCallId]() {
                //previewWidget_->hide();
                previewWidget_->setContainerSize(this->size());
                // since there is the possiblity of image not reloaded properly
                // after rendering reconnect
                LRCInstance::renderer()->connectPreviewRendering();
                ui->videoWidget->connectDistantRendering();

                auto decive = deviceName.toStdString();
                LRCInstance::avModel().switchInputTo(decive);
                LRCInstance::avModel().setCurrentVideoCaptureDevice(decive);
            });
    }

    menu.addSeparator();

    // entire screen share
    auto shareAction = new QAction(tr("Share entire screen"), this);
    menu.addAction(shareAction);
    shareAction->setCheckable(true);
    connect(shareAction, &QAction::triggered,
        [this]() {
            auto screenNumber = qApp->desktop()->screenNumber(this);
            QScreen* screen = qApp->screens().at(screenNumber);
            QRect rect = screen ? screen->geometry() : qApp->primaryScreen()->geometry();
#if defined(Q_OS_WIN) && (PROCESS_DPI_AWARE)
            rect.setSize(Utils::getRealSize(screen));
#endif
            LRCInstance::avModel().setDisplay(screenNumber,
                rect.x(), rect.y(), rect.width(), rect.height()
            );
            sharingEntireScreen_ = true;
        });

    // area of screen share
    auto shareAreaAction = new QAction(tr("Share screen area"), this);
    menu.addAction(shareAreaAction);
    shareAreaAction->setCheckable(true);
    connect(shareAreaAction, &QAction::triggered,
        [this]() {
            SelectAreaDialog selectAreaDialog;
            selectAreaDialog.exec();
            sharingEntireScreen_ = false;
        });

    // share a media file
    auto shareFileAction = new QAction(tr("Share file"), this);
    menu.addAction(shareFileAction);
    shareFileAction->setCheckable(true);
    connect(shareFileAction, &QAction::triggered,
        [this]() {
            QFileDialog fileDialog(this);
            fileDialog.setFileMode(QFileDialog::AnyFile);
            QStringList fileNames;
            if (!fileDialog.exec())
                return;
            fileNames = fileDialog.selectedFiles();
            auto resource = QUrl::fromLocalFile(fileNames.at(0)).toString();
            LRCInstance::avModel().setInputFile(resource.toStdString());
        });

    // possibly select the alternative video sharing device
    switch (activeDevice.type) {
    case lrc::api::video::DeviceType::DISPLAY:
        sharingEntireScreen_ ? shareAction->setChecked(true) : shareAreaAction->setChecked(true);
        break;
    case lrc::api::video::DeviceType::FILE:
        shareFileAction->setChecked(true);
        break;
    default:
        // a camera must have already been selected
        break;
    }

    menu.exec(globalPos);
}

void
VideoView::setupForConversation(const conversation::Info& convInfo)
{
    pConvInfo_ = &convInfo;

    auto callModel = LRCInstance::getCurrentCallModel();

    if (!callModel->hasCall(pConvInfo_->callId)) {
        return;
    }

    auto call = callModel->getCall(pConvInfo_->callId);

    // close chat panel
    emit overlay_->setChatVisibility(false);

    // preview
    LRCInstance::renderer()->connectPreviewRendering();
    Utils::oneShotConnect(LRCInstance::renderer(), &RenderManager::previewFrameUpdated,
        [this] {
            resetPreviewPosition();
        });

    // TODO(atraczyk): videoWidget --> distantRenderer
    QObject::disconnect(ui->videoWidget);
    ui->videoWidget->connectDistantRendering();
    ui->videoWidget->show();

    // setup overlay
    // TODO(atraczyk): all of this could be done with conversation::Info
    // transfer call will only happen in SIP calls
    auto& accInfo = LRCInstance::accountModel().getAccountInfo(pConvInfo_->accountId);
    bool isSIP = accInfo.profileInfo.type == lrc::api::profile::Type::SIP;
    auto& convModel = accInfo.conversationModel;
    auto bestName = QString::fromStdString(Utils::bestNameForConversation(*pConvInfo_, *convModel));
    bool isAudioMuted = call.audioMuted && (call.status != lrc::api::call::Status::PAUSED);
    bool isVideoMuted = call.videoMuted && (call.status != lrc::api::call::Status::PAUSED) && (!call.isAudioOnly);
    bool isRecording = callModel->isRecording(pConvInfo_->callId);
    bool isPaused = call.status == lrc::api::call::Status::PAUSED;
    bool isAudioOny = call.isAudioOnly && call.status != lrc::api::call::Status::PAUSED;
    overlay_->callStarted(pConvInfo_->callId);
    overlay_->setTransferCallAndSIPPanelAvailability(isSIP);
    overlay_->setVideoMuteVisibility(!call.isAudioOnly);
    overlay_->resetOverlay(isAudioMuted, isVideoMuted, isRecording, isPaused, isAudioOny);
    overlay_->setCurrentSelectedCalleeDisplayName(bestName);
    overlay_->setName(bestName);
    // TODO(atraczyk): this should be part of the overlay
    resetAvatarOverlay(call.isAudioOnly);
    if (call.isAudioOnly) {
        audioOnlyAvatar_->writeAvatarOverlay(*pConvInfo_);
    }

    // listen for the end of the call
    QObject::disconnect(callStatusChangedConnection_);
    callStatusChangedConnection_ =
        QObject::connect(callModel,
        &NewCallModel::callStatusChanged,
        this,
        &VideoView::slotCallStatusChanged);
}

void
VideoView::mousePressEvent(QMouseEvent* event)
{
    QPoint clickPosition = event->pos();
    if (!previewWidget_->geometry().contains(clickPosition)) {
        return;
    }

    QLine distance = QLine(clickPosition, previewWidget_->geometry().bottomRight());
    originMouseDisplacement_ = event->pos() - previewWidget_->geometry().topLeft();
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
    draggingPreview_ = true;
    moveAnim_->stop();
}

void
VideoView::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    if (!draggingPreview_) {
        return;
    }

    //Check preview's current central position
    auto previewCenter = previewWidget_->geometry().center();
    auto videoWidgetCenter = ui->videoWidget->rect().center();
    moveAnim_->stop();
    moveAnim_->setStartValue(previewWidget_->geometry());
    PreviewSnap newPreviewLocation;
    if (previewCenter.x() >= videoWidgetCenter.x()) {
        if (previewCenter.y() >= videoWidgetCenter.y()) {
            newPreviewLocation = PreviewSnap::SE;
        } else {
            newPreviewLocation = PreviewSnap::NE;
        }
    } else {
        if (previewCenter.y() >= videoWidgetCenter.y()) {
            newPreviewLocation = PreviewSnap::SW;
        } else {
            newPreviewLocation = PreviewSnap::NW;
        }
    }
    currentPreviewPosition_ = newPreviewLocation;
    QPoint endPoint = getPreviewPosition(currentPreviewPosition_);
    moveAnim_->setEndValue(QRect(endPoint, previewWidget_->size()));
    moveAnim_->start();
    draggingPreview_ = false;
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void
VideoView::mouseMoveEvent(QMouseEvent* event)
{
    if (!draggingPreview_) {
        return;
    }

    QRect previewRect = previewWidget_->geometry();

    if (previewRect.left() > 0
        && previewRect.top() > 0
        && previewRect.right() < width()
        && previewRect.bottom() < height()) {

        previewRect.moveTo(event->pos() - originMouseDisplacement_);
        if (previewRect.left() <= 0) previewRect.moveLeft(1);
        if (previewRect.right() >= width()) previewRect.moveRight(width() - 1);
        if (previewRect.top() <= 0) previewRect.moveTop(1);
        if (previewRect.bottom() >= height()) previewRect.moveBottom(height() - 1);
    }

    previewWidget_->setGeometry(previewRect);
    previewWidget_->forceRepaint();

}

void
VideoView::resetAvatarOverlay(bool isAudioOnly)
{
    audioOnlyAvatar_->setAvatarVisible(isAudioOnly);
    if (isAudioOnly) {
        disconnect(coordinateOverlays_);
        coordinateOverlays_ = connect(overlay_, SIGNAL(HoldStatusChanged(bool)), this, SLOT(slotHoldStatusChanged(bool)));
    } else {
        disconnect(coordinateOverlays_);
    }
}

void
VideoView::slotHoldStatusChanged(bool pauseLabelStatus)
{
    audioOnlyAvatar_->respondToPauseLabel(pauseLabelStatus);
}

QPoint
VideoView::getPreviewPosition(const PreviewSnap snapLocation)
{
    switch (snapLocation) {
    case PreviewSnap::NW:
        return QPoint(
            previewMargin_,
            previewMargin_);
    case PreviewSnap::NE:
        return QPoint(
            this->width() - previewMargin_ - previewWidget_->width(),
            previewMargin_);
    case PreviewSnap::SW:
        return QPoint(
            previewMargin_,
            this->height() - previewMargin_ - previewWidget_->height());
    case PreviewSnap::SE:
        return QPoint(
            this->width() - previewMargin_ - previewWidget_->width(),
            this->height() - previewMargin_ - previewWidget_->height());
    }
}

void
VideoView::resetPreviewPosition()
{
    auto previewImage = LRCInstance::renderer()->getPreviewFrame();
    if (previewImage) {
        previewWidget_->show();
        auto width = previewImage->width();
        auto height = previewImage->height();
        previewWidget_->computeGeometry(width, height);
        QPoint endPoint = getPreviewPosition(currentPreviewPosition_);
        previewWidget_->setGeometry(QRect(endPoint, QSize(width, height)));
    }
}

void
VideoView::keyPressEvent(QKeyEvent* event)
{
    // used to manage DTMF
    // For "#" and "*", qt will automatically read the shift + 3 or 8
    keyPressed_ = event->key();
    QWidget::keyPressEvent(event);
}

void
VideoView::keyReleaseEvent(QKeyEvent* event)
{
    if (keyPressed_ == static_cast<int>(Qt::Key_NumberSign)) {
        LRCInstance::getCurrentCallModel()->playDTMF(pConvInfo_->callId, "#");
    } else if (keyPressed_ == static_cast<int>(Qt::Key_Asterisk)) {
        LRCInstance::getCurrentCallModel()->playDTMF(pConvInfo_->callId, "*");
    } else if (keyPressed_ >= 48 && keyPressed_ <= 57) {
        //enum Qt::Key_0 = 48, QT::Key_9 = 57
        LRCInstance::getCurrentCallModel()->playDTMF(pConvInfo_->callId, std::to_string(keyPressed_ - 48));
    }
    QWidget::keyReleaseEvent(event);
}

void VideoView::paintEvent(QPaintEvent* e)
{
    QWidget::paintEvent(e);
}
