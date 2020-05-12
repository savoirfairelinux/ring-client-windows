
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
import net.jami.JamiTheme 1.0
import net.jami.ConversationsAdapter 1.0
import net.jami.CallAdapter 1.0

import "../../commoncomponents"

Menu {
    id: contextMenu

    property string responsibleAccountId: ""
    property string responsibleConvUid: ""

    property int generalMenuSeparatorCount: 0
    property int commonBorderWidth: 2


    /*
     * All GeneralMenuItems should remain the same width / height
     */
    GeneralMenuItem {
        id: startVideoCallItem

        itemName: qsTr("Start video call")
        topBorderWidth: commonBorderWidth
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth

        onClicked: {
            contextMenu.close()
            ConversationsAdapter.selectConversation(responsibleAccountId,
                                                    responsibleConvUid, false)
            CallAdapter.placeCall()
        }
    }

    GeneralMenuItem {
        id: startAudioCallItem

        itemName: qsTr("Start audio call")
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth

        onClicked: {
            contextMenu.close()
            ConversationsAdapter.selectConversation(responsibleAccountId,
                                                    responsibleConvUid, false)
            CallAdapter.placeAudioOnlyCall()
        }
    }

    GeneralMenuItem {
        id: clearConversationItem

        itemName: qsTr("Clear conversation")
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth

        onClicked: {
            contextMenu.close()
            utilsAdapter.clearConversationHistory(responsibleAccountId,
                                                  responsibleConvUid)
        }
    }

    GeneralMenuItem {
        id: removeContactItem

        itemName: qsTr("Remove contact")
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth

        onClicked: {
            contextMenu.close()
            utilsAdapter.removeConversation(responsibleAccountId,
                                            responsibleConvUid)
        }
    }

    GeneralMenuSeparator {
        preferredWidth: startVideoCallItem.preferredWidth
        preferredHeight: commonBorderWidth

        Component.onCompleted: {
            generalMenuSeparatorCount++
        }
    }

    GeneralMenuItem {
        id: blockContactItem

        itemName: qsTr("Block contact")
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth

        onClicked: {
            contextMenu.close()
            utilsAdapter.removeConversation(responsibleAccountId,
                                            responsibleConvUid, true)
        }
    }

    GeneralMenuSeparator {
        preferredWidth: startVideoCallItem.preferredWidth
        preferredHeight: commonBorderWidth

        Component.onCompleted: {
            generalMenuSeparatorCount++
        }
    }

    GeneralMenuItem {
        id: profileItem

        itemName: qsTr("Profile")
        bottomBorderWidth: commonBorderWidth
        leftBorderWidth: commonBorderWidth
        rightBorderWidth: commonBorderWidth

        onClicked: {
            contextMenu.close()
        }
    }

    background: Rectangle {
        implicitWidth: startVideoCallItem.preferredWidth
        implicitHeight: startVideoCallItem.preferredHeight
                        * (contextMenu.count - generalMenuSeparatorCount)

        border.width: commonBorderWidth
        border.color: JamiTheme.tabbarBorderColor
    }
}
