/***************************************************************************
 * Copyright (C) 2015 by Savoir-faire Linux                                *
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

#include "instantmessagingwidget.h"
#include "ui_instantmessagingwidget.h"

#include <QApplication>
#include <QClipboard>
#include <QMenu>

#include "navstack.h"

#include "media/text.h"
#include "media/textrecording.h"

#include "imdelegate.h"
#include "globalsystemtray.h"

InstantMessagingWidget::InstantMessagingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InstantMessagingWidget)
{
    ui->setupUi(this);

    this->hide();

    imDelegate_ = new ImDelegate();
    ui->messageOutput->setItemDelegate(imDelegate_);
    ui->messageOutput->setContextMenuPolicy(Qt::ActionsContextMenu);
    auto copyAction = new QAction(tr("Copy"), this);
    ui->messageOutput->addAction(copyAction);
    connect(copyAction, &QAction::triggered, [=]() {
        copyToClipboard();
    });
    auto displayDate = new QAction(tr("Display date"), this);
    displayDate->setCheckable(true);
    ui->messageOutput->addAction(displayDate);
    auto displayAuthor = new QAction(tr("Display author"), this);
    displayAuthor->setCheckable(true);
    ui->messageOutput->addAction(displayAuthor);
    auto lamdba = [=](){
        int opts = 0;
        displayAuthor->isChecked() ? opts |= ImDelegate::DisplayOptions::AUTHOR : opts;
        displayDate->isChecked() ? opts |= ImDelegate::DisplayOptions::DATE : opts;
        imDelegate_->setDisplayOptions(static_cast<ImDelegate::DisplayOptions>(opts));
    };
    connect(displayAuthor, &QAction::triggered, lamdba);
    connect(displayDate, &QAction::triggered, lamdba);
}

InstantMessagingWidget::~InstantMessagingWidget()
{
    delete ui;
    delete imDelegate_;
}

void
InstantMessagingWidget::setMediaText(Call *call)
{
    if (call != nullptr) {
        connect(call, SIGNAL(mediaAdded(Media::Media*)),
                this, SLOT(mediaAdd(Media::Media*)));
        Media::Text *textMedia = nullptr;
        if (call->hasMedia(Media::Media::Type::TEXT, Media::Media::Direction::OUT)) {
            textMedia = call->firstMedia<Media::Text>(Media::Media::Direction::OUT);
        } else {
            textMedia = call->addOutgoingMedia<Media::Text>();
        }
        if (textMedia) {
            connect(ui->messageOutput->model(),
                    SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                    ui->messageOutput, SLOT(scrollToBottom()));
            ui->messageOutput->setModel(
                        textMedia->recording()->
                        instantMessagingModel());
            connect(ui->messageInput, &QLineEdit::returnPressed, [=]()
            {
                QMap<QString, QString> messages;
                messages["text/plain"] = ui->messageInput->text();
                textMedia->send(messages);
                ui->messageInput->clear();
            });
        }
    } else {
        ui->messageOutput->disconnect();
        ui->messageInput->disconnect();
    }
}

void
InstantMessagingWidget::mediaAdd(Media::Media *media)
{
    switch(media->type()) {
    case Media::Media::Type::AUDIO:
        break;
    case Media::Media::Type::VIDEO:
        break;
    case Media::Media::Type::TEXT:
        if (media->direction() == Media::Text::Direction::IN) {
            connect(static_cast<Media::Text*>(media),
                    SIGNAL(messageReceived(QMap<QString,QString>)),
                    this,
                    SLOT(onMsgReceived(QMap<QString,QString>)));
            this->show();
        }
        break;
    case Media::Media::Type::FILE:
        break;
    default:
        break;
    }
}

void
InstantMessagingWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Copy)) {
        copyToClipboard();
    }
}

void
InstantMessagingWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    ui->messageInput->setFocus();
}

void
InstantMessagingWidget::copyToClipboard()
{
    auto idx = ui->messageOutput->currentIndex();
    if (idx.isValid()) {
        auto text = ui->messageOutput->model()->data(idx);

        QApplication::clipboard()->setText(text.value<QString>());
    }
}

void
InstantMessagingWidget::on_sendButton_clicked()
{
    emit ui->messageInput->returnPressed();
}

void
InstantMessagingWidget::onMsgReceived(const QMap<QString,QString>& message)
{
    if (!QApplication::activeWindow()) {
        GlobalSystemTray::instance().showMessage("Ring: Message Received", message["text/plain"]);
        QApplication::alert(this, 5000);
    }
    this->show();
}
