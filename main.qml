import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("File List")

    ListView {
        id: listView
        width: parent.width
        height: parent.height
        model: fileModel

        delegate: Item {
            width: listView.width
            height: 40

            Rectangle {
                width: parent.width
                height: parent.height
                color: "lightgrey"
                border.color: "grey"

                Row {
                    spacing: 10
                    anchors.centerIn: parent

                    Text {
                        text: model.name
                    }

                    Text {
                        text: model.size + " bytes"
                    }

                    Text {
                        text: model.status
                        color: "blue"
                        visible: model.status.length > 0
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (model.name.endsWith(".bmp") || model.name.endsWith(".barch")) {
                        fileModel.process(index);
                    } else {
                        errorDialog.message = "Unknown file type";
                        errorDialog.visible = true;
                    }
                }
            }
        }
    }

    Rectangle {
        id: errorDialog
        width: 300
        height: 150
        color: "white"
        border.color: "black"
        visible: false
        anchors.centerIn: parent

        property string message: "error"

        Text {
            id: errorText
            text: errorDialog.message
            anchors.centerIn: parent
        }

        Button {
            id: okButton
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: errorDialog.visible = false
            text: "OK"
        }
    }
}
