/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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

#include "previewrenderer.h"

#include "lrcinstance.h"

PreviewRenderer::PreviewRenderer(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setAntialiasing(true);
    setFillColor(Qt::black);
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
    setPerformanceHint(QQuickPaintedItem::FastFBOResizing);

    previewFrameUpdatedConnection_ = connect(LRCInstance::renderer(),
                                             &RenderManager::previewFrameUpdated,
                                             [this]() { update(QRect(0, 0, width(), height())); });
    previewRenderingStopped_ = connect(LRCInstance::renderer(),
                                       &RenderManager::previewRenderingStopped,
                                       [this]() { update(QRect(0, 0, width(), height())); });
}

PreviewRenderer::~PreviewRenderer()
{
    disconnect(previewFrameUpdatedConnection_);
    disconnect(previewRenderingStopped_);
}

void
PreviewRenderer::paint(QPainter *painter)
{
    auto previewImage = LRCInstance::renderer()->getPreviewFrame();
    if (previewImage) {
        QImage scaledPreview;
        auto aspectRatio = static_cast<qreal>(previewImage->width())
                           / static_cast<qreal>(previewImage->height());
        auto previewHeight = height();
        auto previewWidth = previewHeight * aspectRatio;

        /* Instead of setting fixed size, we could get an x offset for the preview
         * but this would render the horizontal spacers in the parent widget useless.
         * e.g.
         * auto parent = qobject_cast<QWidget*>(this->parent());
         * auto xPos = (parent->width() - previewWidth) / 2;
         * setGeometry(QRect(QPoint(xPos, this->pos().y()), QSize(previewWidth, previewHeight)));
        */
        setWidth(previewWidth);
        setHeight(previewHeight);

        scaledPreview = previewImage->scaled(size().toSize(), Qt::KeepAspectRatio);
        painter->drawImage(QRect(0, 0, scaledPreview.width(), scaledPreview.height()),
                           scaledPreview);
    }
}

VideoCallPreviewRenderer::VideoCallPreviewRenderer(QQuickItem *parent)
    : PreviewRenderer(parent)
{}

VideoCallPreviewRenderer::~VideoCallPreviewRenderer() {}

qreal
VideoCallPreviewRenderer::getPreviewImageScalingFactor()
{
    auto previewImage = LRCInstance::renderer()->getPreviewFrame();
    if (previewImage) {
        return static_cast<qreal>(previewImage->height())
               / static_cast<qreal>(previewImage->width());
    }
    return 1.0;
}

void
VideoCallPreviewRenderer::paint(QPainter *painter)
{
    auto previewImage = LRCInstance::renderer()->getPreviewFrame();

    if (previewImage) {
        emit previewImageAvailable();

        QImage scaledPreview;
        scaledPreview = previewImage->scaled(size().toSize(), Qt::KeepAspectRatio);
        painter->drawImage(QRect(0, 0, scaledPreview.width(), scaledPreview.height()),
                           scaledPreview);
    }
}

PhotoboothPreviewRender::PhotoboothPreviewRender(QQuickItem *parent)
    : PreviewRenderer(parent)
{
    connect(LRCInstance::renderer(), &RenderManager::previewRenderingStopped, [this]() {
        emit hideBooth();
    });
}

PhotoboothPreviewRender::~PhotoboothPreviewRender() {}

QImage
PhotoboothPreviewRender::takePhoto()
{
    if (auto previewImage = LRCInstance::renderer()->getPreviewFrame()) {
        return previewImage->copy();
    }
    return QImage();
}

QString
PhotoboothPreviewRender::takeCroppedPhotoToBase64(int size)
{
    auto image = Utils::cropImage(takePhoto());
    auto avatar = image.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    return QString::fromLatin1(Utils::QImageToByteArray(avatar).toBase64().data());
}

void
PhotoboothPreviewRender::paint(QPainter *painter)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    auto previewImage = LRCInstance::renderer()->getPreviewFrame();
    if (previewImage) {
        QImage scaledPreview;
        scaledPreview = Utils::getCirclePhoto(*previewImage,
                                              height() <= width() ? height() : width());
        painter->drawImage(QRect(0, 0, scaledPreview.width(), scaledPreview.height()),
                           scaledPreview);
    }
}
