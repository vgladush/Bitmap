import QtQuick 2.15

Rectangle {
    id: button
    width: 100
    height: 30
    color: "lightblue"
    border.color: "blue"
    radius: 5

    property alias text: buttonText.text

    signal clicked

    Text {
        id: buttonText
        text: "Button"
        anchors.centerIn: parent
        color: "black"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        onPressed: {
            button.color = "lightgreen"
        }

        onReleased: {
            button.color = "lightblue"
        }

        onClicked: {
            parent.clicked();
        }
    }

    function setText(newText) {
        buttonText.text = newText;
    }
}
