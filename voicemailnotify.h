/**************************************************************************
* Copyright (C) 2015-2019 by Savoir-faire Linux                           *
* Author: Mingrui Zhang   <mingrui.zhang@savoirfairelinux.com>            *
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

// qt
#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

// project
#include "lrcinstance.h"
#include "utils.h"

namespace Ui
{
    class VoicemailNotifyWidget;
}

class VoicemailNotifyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VoicemailNotifyWidget(QWidget *parent = nullptr);
    ~VoicemailNotifyWidget();

    void setCurrentData(int newVM, int oldVM, int newUrgentVM);

private:
    Ui::VoicemailNotifyWidget *ui;
};
