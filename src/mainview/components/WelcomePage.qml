
/*
 * Copyright (C) 2020 by Savoir-faire Linux
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import net.jami.JamiTheme 1.0
import net.jami.UtilsAdapter 1.0

import "../../commoncomponents"

Rectangle {
    id: welcomeRect

    property int currentAccountIndex: 0
    property int buttonPreferredSize: 30

    anchors.fill: parent

    Rectangle {
        id: welcomeRectComponentsGroup

        anchors.centerIn: parent

        width: Math.max(welcomePageGroupPreferedWidth, welcomeRect.width - 100)
        height: mainViewWindow.minimumHeight

        ColumnLayout {
            id: welcomeRectComponentsGroupColumnLayout

            Image {
                id: jamiLogoImage

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: welcomeRectComponentsGroup.width
                Layout.preferredHeight: 100
                Layout.topMargin: 50
                Layout.bottomMargin: 10

                fillMode: Image.PreserveAspectFit
                source: "qrc:/images/logo-jami-standard-coul.png"
                mipmap: true
            }

            Label {
                id: jamiIntroText

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: welcomeRectComponentsGroup.width
                Layout.preferredHeight: 100
                Layout.bottomMargin: 5

                wrapMode: Text.WordWrap
                font.pointSize: JamiTheme.textFontSize + 1

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                text: qsTr("Jami is a free software for universal communication which repects the freedoms and privacy of its user.")
            }

            Label {
                id: jamiShareWithFriendText

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: welcomeRectComponentsGroup.width
                Layout.preferredHeight: 50

                wrapMode: Text.WordWrap
                font.pointSize: JamiTheme.textFontSize - 1

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                visible: accountListModel.data(accountListModel.index(
                                                   currentAccountIndex, 0),
                                               260) === 1
                text: qsTr("This is your ID.\nCopy and share it with your friends")
                color: JamiTheme.faddedFontColor
            }

            Rectangle {
                id: jamiRegisteredNameRect

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: welcomeRectComponentsGroup.width
                Layout.preferredHeight: 65
                Layout.bottomMargin: 5

                visible: accountListModel.data(accountListModel.index(
                                                   currentAccountIndex, 0),
                                               260) === 1

                ColumnLayout {
                    id: jamiRegisteredNameRectColumnLayout

                    spacing: 0

                    Text {
                        id: jamiRegisteredNameText

                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: welcomeRectComponentsGroup.width
                        Layout.preferredHeight: 30

                        font.pointSize: JamiTheme.textFontSize

                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        text: textMetricsjamiRegisteredNameText.elidedText

                        TextMetrics {
                            id: textMetricsjamiRegisteredNameText

                            font: jamiRegisteredNameText.font
                            text: accountListModel.data(
                                      accountListModel.index(
                                          currentAccountIndex, 0), 258)
                            elideWidth: welcomeRectComponentsGroup.width
                            elide: Qt.ElideMiddle
                        }
                    }

                    RowLayout {
                        id: jamiRegisteredNameRowLayout

                        Layout.alignment: Qt.AlignCenter
                        Layout.preferredWidth: buttonPreferredSize * 2 + 20
                        Layout.preferredHeight: 30

                        HoverableButton {
                            id: copyRegisterednameButton

                            Layout.alignment: Qt.AlignCenter
                            Layout.preferredWidth: buttonPreferredSize
                            Layout.preferredHeight: buttonPreferredSize

                            radius: 30
                            source: "qrc:/images/icons/ic_content_copy.svg"

                            onClicked: {
                                UtilsAdapter.setText(
                                            textMetricsjamiRegisteredNameText.text)
                            }
                        }

                        HoverableButton {
                            id: qrCodeGenerateButton

                            Layout.alignment: Qt.AlignCenter
                            Layout.preferredWidth: buttonPreferredSize
                            Layout.preferredHeight: buttonPreferredSize

                            radius: 30
                            source: "qrc:/images/qrcode.png"

                            onClicked: {
                                qrDialog.open()
                            }
                        }
                    }
                }
            }

            Image {
                id: sipImage

                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: 200
                Layout.preferredHeight: 200


                /*
                 * Check if account type is ring.
                 */
                visible: accountListModel.data(accountListModel.index(
                                                   currentAccountIndex, 0),
                                               260) === 1 ? false : true
                fillMode: Image.PreserveAspectFit
                source: "qrc:/images/testSIPImage.png"


                /*
                 * Requested size.
                 */
                sourceSize.width: 200
                sourceSize.height: 200
                mipmap: true
            }
        }

        HoverableButton {
            id: aboutButton

            anchors.horizontalCenter: welcomeRectComponentsGroup.horizontalCenter
            anchors.bottom: welcomeRectComponentsGroup.bottom
            anchors.bottomMargin: 5

            width: 80
            height: buttonPreferredSize

            text: qsTr("About")
            fontPointSize: JamiTheme.textFontSize
            radius: 10

            onClicked: {
                aboutPopUpDialog.open()
            }
        }
    }

    CustomBorder {
        commonBorder: false
        lBorderwidth: 1
        rBorderwidth: 0
        tBorderwidth: 0
        bBorderwidth: 0
        borderColor: JamiTheme.tabbarBorderColor
    }
}