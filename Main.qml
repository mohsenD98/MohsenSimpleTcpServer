import QtQuick

Window {
  width: 400
  height: 150
  visible: true
  title: qsTr("Hello World")
  y:200

  Text{
    anchors.centerIn: parent
    text: "Sending data ..."
    font.pixelSize: 24
  }
}
