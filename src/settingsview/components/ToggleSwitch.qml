import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.14
import QtQuick.Controls.Styles 1.4

RowLayout {
    property string labelText: value
    property int widthOfSwitch: 50
    property int heightOfSwitch: 10
    property int heightOfLayout: 30
    property int fontPointSize: 13

    property alias toggleSwitch: switchOfLayout
    property alias checked: switchOfLayout.checked

    signal switchToggled

    spacing: 18
    Layout.fillWidth: true
    Layout.maximumHeight: 30

    Switch {
        id: switchOfLayout
        Layout.alignment: Qt.AlignVCenter

        Layout.maximumWidth: widthOfSwitch
        Layout.preferredWidth: widthOfSwitch
        Layout.minimumWidth: widthOfSwitch

        Layout.minimumHeight: heightOfSwitch
        Layout.preferredHeight: heightOfSwitch
        Layout.maximumHeight: heightOfSwitch

        onToggled: {
            switchToggled()
        }
    }

    Label {
        Layout.fillWidth: true

        Layout.minimumHeight: heightOfLayout
        Layout.preferredHeight: heightOfLayout
        Layout.maximumHeight: heightOfLayout

        text: qsTr(labelText)
        font.pointSize: fontPointSize
        font.kerning: true

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }
}