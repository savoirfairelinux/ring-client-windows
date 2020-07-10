/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
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

import QtQuick 2.15
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import net.jami.Models 1.0

import "../constant"
/*
 * Account Migration Dialog for migrating account
 */

Dialog {
    id: accountMigrationDialog

    visible: false

    anchors.centerIn: parent.Center
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    contentItem: Rectangle{
        implicitWidth: 455
        implicitHeight: 594

        StackLayout{
            id: stackedWidget
            anchors.fill: parent

            currentIndex: 0

            Rectangle{
                id: accountMigrationPage

                Layout.fillWidth: true
                Layout.fillHeight: true

                Layout.leftMargin: 11
                Layout.rightMargin: 11
                Layout.topMargin: 11
                Layout.bottomMargin: 11
            }

            Rectangle{
                id: migrationWaitingPage

                Layout.fillWidth: true
                Layout.fillHeight: true

                Layout.leftMargin: 11
                Layout.rightMargin: 11
                Layout.topMargin: 11
                Layout.bottomMargin: 11

                ColumnLayout{
                    anchors.fill: parent
                    spacing: 7

                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                    Item{
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Layout.preferredHeight: 211
                    }

                    RowLayout{
                        spacing: 7

                        Layout.fillWidth: true

                        Item{
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            Layout.minimumWidth: 20
                        }

                        Label{
                            id: spinnerLabel

                            Layout.alignment: Qt.AlignHCenter

                            Layout.maximumWidth: 200
                            Layout.preferredWidth: 200
                            Layout.minimumWidth: 200

                            Layout.maximumHeight: 200
                            Layout.preferredHeight: 200
                            Layout.minimumHeight: 200

                            background: Rectangle {
                                anchors.fill: parent
                                AnimatedImage {
                                    id: spinnerMovie

                                    anchors.fill: parent

                                    source: "qrc:/images/jami_eclipse_spinner.gif"

                                    playing: spinnerLabel.visible
                                    paused: false
                                    fillMode: Image.PreserveAspectFit
                                    mipmap: true
                                }
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            Layout.minimumWidth: 20
                        }
                    }

                    Item{
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Layout.preferredHeight: 211
                    }

                    Label{
                        id: progressLabel

                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillWidth: true

                        text: qsTr("Migrating your Jami account...")
                        font.pointSize: 11
                        font.kerning: true
                    }

                    Item{
                        Layout.fillWidth: true

                        Layout.minimumHeight: 20
                        Layout.preferredHeight: 20
                        Layout.maximumHeight: 20
                    }
                }
            }
        }
    }
}
