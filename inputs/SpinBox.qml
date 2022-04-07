import QtQuick 2.8
import QtQuick.Controls 2.0

SpinBox {
    id: spinbox
    width: parent.width
    from: 0
    value: 110
    to: 100 * 100
    stepSize: 1
    anchors.centerIn: parent

    property int decimals: 2
    property real realValue: value

    Component.onCompleted: formBuilderPage.result[name] = spinbox.value;

    validator: DoubleValidator {
        bottom: Math.min(spinbox.from, spinbox.to)
        top:  Math.max(spinbox.from, spinbox.to)
    }

    textFromValue: function(value, locale) {
        return Number(value).toLocaleString(locale, 'f', spinbox.decimals)
    }

    valueFromText: function(text, locale) {
        return Number.fromLocaleString(locale, text)
    }

    onValueModified: formBuilderPage.result[name] = spinbox.value;
}
