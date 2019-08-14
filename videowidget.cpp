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

#include "videowidget.h"

#include "utils.h"

VideoWidget::VideoWidget(QWidget* parent) :
    QWidget(parent)
  , isPreviewDisplayed_(true)
  , fullPreview_(false)
{
    QPalette pal(palette());
    pal.setColor(QPalette::Background, Qt::black);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
}

VideoWidget::~VideoWidget()
{}

void
VideoWidget::slotRendererStarted(const std::string& id)
{
    Q_UNUSED(id);

    QObject::disconnect(rendererConnections_.started);

    // only one videowidget will be used at the same time
    if (not isVisible())
        return;

    this->show();

    resetPreview_ = true;

    QObject::disconnect(rendererConnections_.updated);
    rendererConnections_.updated = connect(
        &LRCInstance::avModel(),
        &lrc::api::AVModel::frameUpdated,
        [this](const std::string& id) {
            auto avModel = &LRCInstance::avModel();
            auto renderer = &avModel->getRenderer(id);
            if (!renderer->isRendering()) {
                return;
            }
            using namespace lrc::api::video;
            if (id == PREVIEW_RENDERER_ID) {
                previewRenderer_ = const_cast<Renderer*>(renderer);
            } else {
                distantRenderer_ = const_cast<Renderer*>(renderer);
            }
            renderFrame(id);
        });

    QObject::disconnect(rendererConnections_.stopped);
    rendererConnections_.stopped = connect(
        &LRCInstance::avModel(),
        &lrc::api::AVModel::rendererStopped,
        [this](const std::string& id) {
            QObject::disconnect(rendererConnections_.updated);
            QObject::disconnect(rendererConnections_.stopped);
            using namespace lrc::api::video;
            if (id == PREVIEW_RENDERER_ID) {
                previewRenderer_ = nullptr;
            } else {
                distantRenderer_ = nullptr;
            }
            repaint();
        });
}

void
VideoWidget::renderFrame(const std::string& id)
{
    auto avModel = &LRCInstance::avModel();
    using namespace lrc::api::video;
    auto renderer = &avModel->getRenderer(id);
    if (renderer && renderer->isRendering()) {
        {
            QMutexLocker lock(&mutex_);
            auto tmp  = renderer->currentFrame();
            if (tmp.storage.size()) {
                using namespace lrc::api::video;
                if (id == PREVIEW_RENDERER_ID) {
                    previewFrame_ = tmp;
                } else {
                    distantFrame_ = tmp;
                }
            }
        }
        update();
    }
}

void
VideoWidget::slotToggleFullScreenClicked()
{
    this->update();
}

void
VideoWidget::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (distantRenderer_) {
        {
            QMutexLocker lock(&mutex_);
            if (distantFrame_.storage.size() != 0
                && distantFrame_.storage.size() == (unsigned int)(distantRenderer_->size().height() * distantRenderer_->size().width() * 4)) {
                frameDistant_ = std::move(distantFrame_.storage);
                distantImage_.reset(
                    new QImage((uchar*)frameDistant_.data(),
                        distantRenderer_->size().width(),
                        distantRenderer_->size().height(),
                        QImage::Format_ARGB32_Premultiplied));
            }
        }
        if (distantImage_) {
            auto scaledDistant = distantImage_->scaled(size(), Qt::KeepAspectRatio);
            auto xDiff = (width() - scaledDistant.width()) / 2;
            auto yDiff = (height() - scaledDistant.height()) / 2;
            painter.drawImage(QRect(
                                  xDiff, yDiff, scaledDistant.width(), scaledDistant.height()),
                scaledDistant);
        }
    }
    if ((previewRenderer_ && isPreviewDisplayed_) || (photoMode_ && hasFrame_)) {
        QMutexLocker lock(&mutex_);
        if (previewFrame_.storage.size() != 0
            && previewFrame_.storage.size() == (unsigned int)(previewRenderer_->size().height() * previewRenderer_->size().width() * 4)) {
            framePreview_ = std::move(previewFrame_.storage);
            previewImage_.reset(
                new QImage((uchar*)framePreview_.data(),
                    previewRenderer_->size().width(),
                    previewRenderer_->size().height(),
                    QImage::Format_ARGB32_Premultiplied));
            hasFrame_ = true;
        }
        if (previewImage_) {
            if (resetPreview_) {
                auto previewHeight = fullPreview_ ? height() : height() / 6;
                auto previewWidth = fullPreview_ ? width() : width() / 6;
                QImage scaledPreview;
                if (photoMode_)
                    scaledPreview = Utils::getCirclePhoto(*previewImage_, previewHeight);
                else
                    scaledPreview = previewImage_->scaled(previewWidth, previewHeight, Qt::KeepAspectRatio);
                auto xDiff = (previewWidth - scaledPreview.width()) / 2;
                auto yDiff = (previewHeight - scaledPreview.height()) / 2;
                auto xPos = fullPreview_ ? xDiff : width() - scaledPreview.width() - previewMargin_;
                auto yPos = fullPreview_ ? yDiff : height() - scaledPreview.height() - previewMargin_;
                previewGeometry_.setRect(xPos, yPos, scaledPreview.width(), scaledPreview.height());

                QBrush brush(scaledPreview);
                brush.setTransform(QTransform::fromTranslate(previewGeometry_.x(),
                    previewGeometry_.y()));
                QPainterPath previewPath;
                previewPath.addRoundRect(previewGeometry_, 25);
                painter.fillPath(previewPath, brush);

                resetPreview_ = false;
            }

            QImage scaledPreview;
            if (photoMode_) {
                scaledPreview = Utils::getCirclePhoto(*previewImage_, previewGeometry_.height());
            } else {
                scaledPreview = previewImage_->scaled(previewGeometry_.width(),
                    previewGeometry_.height(),
                    Qt::KeepAspectRatio);
            }
            previewGeometry_.setWidth(scaledPreview.width());
            previewGeometry_.setHeight(scaledPreview.height());

            QBrush brush(scaledPreview);
            brush.setTransform(QTransform::fromTranslate(previewGeometry_.x(),
                previewGeometry_.y()));
            QPainterPath previewPath;
            previewPath.addRoundRect(previewGeometry_, 25);
            painter.fillPath(previewPath, brush);
        }
    } else if (photoMode_) {
        paintBackgroundColor(&painter, Qt::black);
    }
    painter.end();
}

void
VideoWidget::paintBackgroundColor(QPainter* painter, QColor color)
{
    QImage black(1, 1, QImage::Format_ARGB32);
    black.fill(color);
    QImage scaledPreview = Utils::getCirclePhoto(black, height());
    previewGeometry_.setWidth(scaledPreview.width());
    previewGeometry_.setHeight(scaledPreview.height());
    painter->drawImage(previewGeometry_, scaledPreview);
}

void VideoWidget::movePreview(TargetPointPreview typeOfMove)
{
    QRect& previewRect = getPreviewRect();
    switch (typeOfMove)
    {
    case topRight:
        previewRect.moveTopRight(QPoint(width() - previewMargin_, previewMargin_));
        break;
    case topLeft:
        previewRect.moveTopLeft(QPoint(previewMargin_, previewMargin_));
        break;
    case bottomRight:
        previewRect.moveBottomRight(QPoint(width() - previewMargin_, height() - previewMargin_));
        break;
    case bottomLeft:
        previewRect.moveBottomLeft(QPoint(previewMargin_, height() - previewMargin_));
        break;
    case top:
        previewRect.moveTop(previewMargin_);
        break;
    case right:
        previewRect.moveRight(previewMargin_);
        break;
    case bottom:
        previewRect.moveBottom(previewMargin_);
        break;
    case left:
        previewRect.moveLeft(previewMargin_);
        break;

        default:
        break;

    }
}

void
VideoWidget::connectRendering()
{
    rendererConnections_.started = connect(
        &LRCInstance::avModel(),
        SIGNAL(rendererStarted(const std::string&)),
        this,
        SLOT(slotRendererStarted(const std::string&))
    );
}

void
VideoWidget::setPreviewDisplay(bool display)
{
    isPreviewDisplayed_ = display;
}

void
VideoWidget::setIsFullPreview(bool full)
{
    fullPreview_ = full;
}

QImage
VideoWidget::takePhoto()
{
    if (previewImage_) {
        return previewImage_.get()->copy();
    }
    return QImage();
}

void
VideoWidget::setPhotoMode(bool isPhotoMode)
{

    photoMode_ = isPhotoMode;
    auto color = isPhotoMode ? Qt::transparent : Qt::black;

    QPalette pal(palette());
    pal.setColor(QPalette::Background, color);
    setAutoFillBackground(true);
    setPalette(pal);
}
