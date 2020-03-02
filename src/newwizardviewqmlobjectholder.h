/***************************************************************************
* Copyright (C) 2019-2020 by Savoir-faire Linux                            *
* Author: Yang Wang <yang.wang@savoirfairelinux.com>                       *
*                                                                          *
* This program is free software; you can redistribute it and/or modify     *
* it under the terms of the GNU General Public License as published by     *
* the Free Software Foundation; either version 3 of the License, or        *
* (at your option) any later version.                                      *
*                                                                          *
* This program is distributed in the hope that it will be useful,          *
* but WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
* GNU General Public License for more details.                             *
*                                                                          *
* You should have received a copy of the GNU General Public License        *
* along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
***************************************************************************/

#pragma once

#include <QLabel>
#include <QObject>
#include <QTimer>
#include <QMap>

#include <map>

#include "lrcinstance.h"

#include "namedirectory.h"

using namespace lrc::api::account;

class NewWizardViewQmlObjectHolder : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

    //Q_PROPERTY(WizardMode wizardMode READ getWizardMode WRITE setWizardMode NOTIFY wizardModeChanged)
    //Q_PROPERTY(bool registrationStateOk READ getRegistrationStateOk WRITE setRegistrationStateOk NOTIFY registrationStateOkChanged)
    //Q_PROPERTY(QString fileToImport READ getFileToImport WRITE setFileToImport NOTIFY fileToImportChanged)
    //Q_PROPERTY(QString registeredName READ getRegisteredName WRITE setRegisteredName NOTIFY registeredNameChanged)

public:
    explicit NewWizardViewQmlObjectHolder(QObject *parent = nullptr);

public:
    // functions to call in QML
    Q_INVOKABLE void setNewWizardViewQmlObject(QObject *obj);

    // function wrappers for for handling signals, slots or c++ calls
    Q_INVOKABLE void createAccount(bool isConnectingToManager, bool isRing, bool isCreating);
    Q_INVOKABLE void settingsNeverShowAgainBox(bool checked);
    Q_INVOKABLE bool exportToFile(const QString accountId,
                                  const QString path,
                                  const QString password = {});

    Q_INVOKABLE void clearInputPara();
    Q_INVOKABLE void modifyInputPara(QString key, QString value,bool toClear=false);
    Q_INVOKABLE bool hasPassword();

    Q_INVOKABLE void createJamiAccount(const QVariantMap &settings, bool isCreating);
    Q_INVOKABLE void createSIPAccount(const QVariantMap &settings);
    Q_INVOKABLE void createJAMSAccount(const QVariantMap &settings);


    void connectFailure();
signals:
    // show main view signal
    void showMainViewWindow();

    // object holder signals
    // page indexs:
    // welcomePageStackId: 0
    // createAccountPageId : 1
    // createSIPAccountPageId : 2
    // importFromBackupPageId : 3
    // backupKeysPageId : 4
    // importFromDevicePageId : 5
    // connectToAccountManagerPageId : 6
    // spinnerPageId : 7
    //void changePage(int pageIndex);
    void reportFailure();
    void accountAdded(bool showBackUp);
    //void updateQuickWindow();

private:
    QObject *newWizardViewQmlObject;

    void printSettingParaMap(const QVariantMap & map);
};
