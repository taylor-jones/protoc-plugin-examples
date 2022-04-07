import QtQuick 2.8
import QtQuick.Controls 2.2
import QtQuick.Window 2.2

ApplicationWindow {
    visible: true
    width: Screen.width / 2.5
    height: Screen.height * 0.90
    title: qsTr("Hello World!")

    Component.onCompleted: {
        // when runnig in desktop mode, centralize the application window.
        setX(Screen.width / 2 - width / 2)
        setY(Screen.height / 2 - height / 2)
    }

    // jsonFile is provided from main.cc
    property var formContent: JsonReader.ReadJsonFile(jsonFile)

    FormBuilderPage {
        id: page
        form: formContent
    }

    StackView {
        id: pageStack
        initialItem: page
        anchors.fill: parent
    }
}
