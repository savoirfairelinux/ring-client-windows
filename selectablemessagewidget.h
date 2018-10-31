/***************************************************************************
 * Copyright (C) 2018 by Savoir-faire Linux                                *
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
#pragma once

#include <QWidget>
#include <QItemDelegate>

namespace Ui {
class SelectableMessageWidget;
}

class SelectableMessageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SelectableMessageWidget(  QModelIndex index,
                                        QWidget* parent = 0);
    ~SelectableMessageWidget();

protected:
    void leaveEvent(QEvent* event);

private:
    Ui::SelectableMessageWidget* ui;
    void paintEvent(QPaintEvent* e);

    QWidget* parent_;
    QModelIndex index_;

signals:
    void exited() const;
};