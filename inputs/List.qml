import QtQuick 2.8
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3


Rectangle {
    property string name

    width: parent.width;

    ListModel { id: listModel }

    // The delegate for each fruit in the model:
    Component {
        id: listDelegate

        Item {
            id: delegateItem
            width: listView.width; height: 55
            clip: true

            Row {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 10

                Column {
                    Image {
                        source: "qrc:/assets/up-arrow.png"
                        MouseArea { anchors.fill: parent; onClicked: listModel.move(index, index-1, 1) }
                    }
                    Image {
                        source: "qrc:/assets/down-arrow.png"
                        MouseArea { anchors.fill: parent; onClicked: listModel.move(index, index+1, 1) }
                    }
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        text: name
                        font.pixelSize: 15
                        color: "white"
                     }
                    Row {
                        spacing: 5
                        Repeater {
                            model: attributes
                            Text {
                                text: description;
                                color: "White"
                            }
                        }
                    }
                }
            }

            Row {
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                spacing: 10

                Button {
                    anchors.verticalCenter: parent.verticalCenter
                    icon.source: "qrc:/assets/add.png"
                    onPressAndHold: listModel.setProperty(index, "cost", cost + 0.25)
                }

                Text {
                    id: costText
                    anchors.verticalCenter: parent.verticalCenter
                    text: '$' + Number(cost).toFixed(2)
                    font.pixelSize: 15
                    color: "white"
                    font.bold: true
                }

                Button {
                    anchors.verticalCenter: parent.verticalCenter
                    icon.source: "qrc:/assets/minus.png"
                    onPressAndHold: listModel.setProperty(index, "cost", Math.max(0,cost-0.25))
                }

                //  Image {
                //      source: "content/pics/list-delete.png"
                //      MouseArea { anchors.fill:parent; onClicked: listModel.remove(index) }
                //  }
             }

             // Animate adding and removing of items:
            ListView.onAdd: SequentialAnimation {
                PropertyAction {
                    target: delegateItem;
                    property: "height";
                    value: 0
                }
                NumberAnimation {
                    target: delegateItem;
                    property: "height";
                    to: 55;
                    duration: 250;
                    easing.type: Easing.InOutQuad
                }
            }

            ListView.onRemove: SequentialAnimation {
                PropertyAction {
                    target: delegateItem;
                    property: "ListView.delayRemove";
                    value: true
                }
                NumberAnimation {
                    target: delegateItem;
                    property: "height";
                    to: 0;
                    duration: 250;
                    easing.type: Easing.InOutQuad
                }
                // Make sure delayRemove is set back to false so that the item can be destroyed
                PropertyAction {
                    target: delegateItem;
                    property: "ListView.delayRemove";
                    value: false
                }
            }
        }
    }

    ListView {
        id: listView
        anchors.fill: parent
        model: listModel
        delegate: listDelegate
    }

    Row {
        anchors {
            left: parent.left;
            bottom: parent.bottom;
            margins: 20
        }

        spacing: 10

        Button {
            text: "Add an item"
            onClicked: {
                listModel.append({
                    "name": "Pizza Margarita",
                    "cost": 5.95,
                    "attributes": [{"description": "Cheese"}, {"description": "Tomato"}]
                })
            }
        }

        Button {
            text: "Remove all items"
            onClicked: listModel.clear()
        }
    }
}

