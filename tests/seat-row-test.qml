import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import "../src/app"

Item {
    width: 600
    height: 800

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        Kirigami.Heading {
            text: "compact content"
        }

        TimelineDelegateSeatRow {
            TimelineDelegateSeatRowLabel {
                text: "Coach: <b>13</b>"
            }
            Kirigami.Separator {
                Layout.fillHeight: true
            }
            TimelineDelegateSeatRowLabel {
                text: "Seat: <b>42</b>"
            }
            Kirigami.Separator {
                Layout.fillHeight: true
            }
            TimelineDelegateSeatRowLabel {
                text: "Class: <b>2</b>"
                lowPriority: true
            }
        }

        Kirigami.Heading {
            text: "long content"
        }

        TimelineDelegateSeatRow {
            TimelineDelegateSeatRowLabel {
                text: "Wagen: <b>13</b>"
            }
            Kirigami.Separator {
                Layout.fillHeight: true
            }
            TimelineDelegateSeatRowLabel {
                text: "Sitzplätze: <b>108, 109, 110, 111</b>"
            }
            Kirigami.Separator {
                Layout.fillHeight: true
            }
            TimelineDelegateSeatRowLabel {
                text: "Klasse: <b>2</b>"
                lowPriority: true
            }
        }
    }

}
