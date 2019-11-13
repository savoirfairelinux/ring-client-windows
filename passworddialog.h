/**************************************************************************
* Copyright (C) 2019-2019 by Savoir-faire Linux                           *
* Author: Isa Nanic <isa.nanic@savoirfairelinux.com>                      *
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

#include <QDialog>
#include <QMovie>
#include <QVariant>

namespace Ui {
    class PasswordDialog;
}

enum class PasswordEnteringPurpose{
    ChangePassword,
    ExportAccount
};

class PasswordDialogQMLControllerObject;

class PasswordDialog : public QDialog
{
    Q_OBJECT
    friend class PasswordDialogQMLControllerObject;

public:
    PasswordDialog(QWidget* parent = nullptr, PasswordEnteringPurpose purpose = PasswordEnteringPurpose::ChangePassword);
    ~PasswordDialog();

    static const int SuccessCode = 200;

    void setExportPath(const std::string& path) { path_ = path; }

    bool savePassword(QString currentPasswordEditPassword, QString passwordEditPassword);

    bool exportAccount(QString currentPasswordEditPassword);

protected:
    void showEvent(QShowEvent* event) override;

private:
    Ui::PasswordDialog* ui;

    PasswordEnteringPurpose purpose_ { PasswordEnteringPurpose::ChangePassword };
    std::string path_;
    QMovie* spinnerMovie_;
    PasswordDialogQMLControllerObject* quickUiRootObj_;


};

class PasswordDialogQMLControllerObject : public QObject
{
    Q_OBJECT
public:
    explicit PasswordDialogQMLControllerObject(PasswordDialog* parent = nullptr);
    ~PasswordDialogQMLControllerObject();

    Q_ENUMS(PasswordEnteringPurpose)

    Q_INVOKABLE void cancelBtnClick();
    Q_INVOKABLE QVariant savePassword(QVariant currentPasswordEditPassword, QVariant passwordEditPassword);
    Q_INVOKABLE QVariant exportAccount(QVariant currentPasswordEditPassword);
};