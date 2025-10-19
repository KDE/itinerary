import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Kirigami.Page {
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    readonly property int year: (new Date()).getFullYear();

    component WrappedPage : Controls.Page {
        footer: Item {
            height: 200
            width: parent.width

            Image {
                y: 40
                height: 160
                width: Math.min(parent.width, 400)
                anchors.horizontalCenter: parent.horizontalCenter
                sourceSize.height: 160
                source: Qt.resolvedUrl("wrapped-banner.svg")
                fillMode: Image.PreserveAspectCrop
            }
        }
    }

    component ContinueButton : Controls.Button {
        text: i18nc("@action:button", "Continue")
        onClicked: swipeView.currentIndex++
    }

    StatisticsModel {
        id: model
        reservationManager: ReservationManager
        tripGroupManager: TripGroupManager
        transferManager: TransferManager
        distanceFormat: Settings.distanceFormat
    }

    StatisticsTimeRangeModel {
        id: timeRangeModel
        reservationManager: ReservationManager

        Component.onCompleted: {
            const begin = timeRangeModel.data(timeRangeModel.index(0, 0), StatisticsTimeRangeModel.BeginRole);
            const end = timeRangeModel.data(timeRangeModel.index(0, 0), StatisticsTimeRangeModel.EndRole);
            model.setTimeRange(begin, end);
        }
    }

    contentItem: Controls.SwipeView {
        id: swipeView

        WrappedPage {
            Controls.Label {
                id: welcomeTitle

                text: i18nc("@title", "Here is how you travelled in <br /> %1", "" + year)
                wrapMode: Text.WordWrap
                font.pointSize: 20
                padding: Kirigami.Units.largeSpacing
                topPadding: Kirigami.Units.largeSpacing * 2
                horizontalAlignment: Text.AlignHCenter
                width: parent.width
                anchors {
                    top: parent.top
                    topMargin: Kirigami.Units.gridUnit * 4
                }
            }

            ContinueButton {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    top: welcomeTitle.bottom
                    topMargin: Kirigami.Units.gridUnit
                }
            }
        }

        WrappedPage {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.largeSpacing

                RowLayout {
                    Layout.fillWidth: true

                    Controls.Label {
                        font.pointSize: 17
                        text: model.totalDistance.label
                    }

                    Controls.Label {
                        text: model.totalDistance.value
                        font.pointSize: 25
                    }
                }
            }
        }

        WrappedPage {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.largeSpacing

                Repeater {
                    model: 12

                    RowLayout {
                        id: monthDelegate

                        required property int modelData

                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.largeSpacing
                        Layout.rightMargin: Kirigami.Units.largeSpacing

                        spacing: Kirigami.Units.smallSpacing

                        Controls.Label {
                            text: Qt.locale().standaloneMonthName(monthDelegate.modelData, Locale.LongFormat)
                            Layout.fillWidth: true
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                        }

                        Rectangle {
                            color: "#DD00FF"

                            Layout.preferredWidth: (parent.width - Kirigami.Units.gridUnit * 4 - monthDelegate.spacing) * monthDelegate.modelData / 12
                            Layout.preferredHeight: Kirigami.Units.largeSpacing
                        }
                    }
                }
            }
        }
    }
}
