/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.i18n.localeData
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.itinerary

FormCard.FormCard {
    id: root
    width: ListView.view.width

    required property var tripGroup
    required property string tripGroupId
    required property int rangeType

    property TripGroupController controller: TripGroupController {
        id: _controller
        tripGroupModel: TripGroupModel
        tripGroupId: root.tripGroupId
        weatherForecastManager: WeatherForecastManager
        homeCountryIsoCode: Settings.homeCountryIsoCode
        homeCurrency: Country.fromAlpha2(Settings.homeCountryIsoCode).currencyCode
    }

    signal removeTrip(tripGroupId: var)

    function clicked() {
        if (root.rangeType === TimelineElement.SelfContained) {
            TripGroupProxyModel.expand(root.tripGroupId);
        } else {
            TripGroupProxyModel.collapse(root.tripGroupId);
        }
    }

    FormCard.AbstractFormDelegate {
        onClicked: root.clicked()
        background: Rectangle {
            id: headerBackground
            color: Kirigami.Theme.backgroundColor
            Kirigami.Theme.colorSet: Kirigami.Theme.Header
            Kirigami.Theme.inherit: false
        }
        contentItem: RowLayout {
            id: headerLayout
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: switch (rangeType) {
                    case TimelineElement.SelfContained: return "go-next-symbolic";
                    case TimelineElement.RangeBegin: return "go-down-symbolic";
                    case TimelineElement.RangeEnd: return "go-up-symbolic";
                }
                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                Layout.preferredHeight: Layout.preferredWidth
                color: Kirigami.Theme.textColor
                isMask: true
            }
            QQC2.Label {
                id: headerLabel
                text: root.rangeType === TimelineElement.RangeEnd ? i18n("End: %1", tripGroup.name) : i18n("Trip: %1", tripGroup.name)
                color: Kirigami.Theme.textColor
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                Layout.fillWidth: true
                Accessible.ignored: true
            }
        }
    }
    FormCard.AbstractFormDelegate {
        id: content
        // hide content entirely in the header-only end elements
        visible: root.rangeType !== TimelineElement.RangeEnd
        onClicked: root.clicked()

        contentItem: Item {
            implicitHeight: contentLayout.implicitHeight

            ColumnLayout {
                id: contentLayout
                spacing: Kirigami.Units.smallSpacing

                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }

                QQC2.Label {
                    Layout.fillWidth: true
                    text: i18np("Date: %2 (one day)", "Date: %2 (%1 days)",
                            Math.ceil((root.tripGroup.endDateTime.getTime() - root.tripGroup.beginDateTime.getTime()) / (1000 * 3600 * 24)),
                            Localizer.formatDateTime(root.tripGroup, "beginDateTime"))
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: _controller.weatherForecast.valid

                    Kirigami.Icon {
                        source: _controller.weatherForecast.symbolIconName
                        width: Kirigami.Units.iconSizes.small
                        height: width
                    }

                    QQC2.Label {
                        text: i18nc("temperature range", "%1 / %2",  Localizer.formatTemperature(_controller.weatherForecast.minimumTemperature),
                                                                    Localizer.formatTemperature(_controller.weatherForecast.maximumTemperature))
                        Accessible.ignored: !parent.visible
                        Layout.fillWidth: true
                    }
                }

                Repeater {
                    model: _controller.locationInformation

                    QQC2.Label {
                        Layout.fillWidth: true
                        text: {
                            if (modelData.powerPlugCompatibility == LocationInformation.PartiallyCompatible) {
                                if (modelData.powerPlugTypes.length === 0) {
                                    return i18n("%1: some incompatible power sockets (%2)", Country.fromAlpha2(modelData.isoCode).name, modelData.powerSocketTypes);
                                } else {
                                    return i18n("%1: some incompatible power plugs (%2)", Country.fromAlpha2(modelData.isoCode).name, modelData.powerPlugTypes);
                                }
                            } else {
                                return i18n("%1: no compatible power plugs (%2)", Country.fromAlpha2(modelData.isoCode).name, modelData.powerSocketTypes);
                            }
                        }
                        color: modelData.powerPlugCompatibility == LocationInformation.PartiallyCompatible ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.negativeTextColor
                        wrapMode: Text.WordWrap
                    }
                }

                QQC2.Label {
                    text: _controller.currencies.length > 0 ? i18np("Currency: %2", "Currencies: %2", _controller.currencies.length, _controller.currencies.join(", ")) : ""
                    visible: _controller.currencies.length > 0
                    Accessible.ignored: !visible
                    Layout.fillWidth: true
                }

                Kirigami.Separator {
                    visible: root.rangeType === TimelineElement.RangeBegin
                    Layout.fillWidth: true
                }

                RowLayout {
                    visible: root.rangeType === TimelineElement.RangeBegin
                    Layout.fillWidth: true
                    Item {
                        Layout.fillWidth: true
                    }
                    QQC2.ToolButton {
                        icon.name: "document-edit"
                        onClicked: {
                            tripGroupEditor.tripGroupId = root.tripGroupId;
                            tripGroupEditor.tripGroup = root.tripGroup;
                            tripGroupEditor.open();
                        }
                        text: i18n("Edit…")
                        display: QQC2.AbstractButton.IconOnly
                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    }
                    QQC2.ToolButton {
                        icon.name: "export-symbolic"
                        onClicked: {
                            exportTripGroupDialog.tripGroupId = root.tripGroupId
                            exportTripGroupDialog.suggestedFileName = root.tripGroup.slugName
                            exportTripGroupDialog.open()
                        }
                        text: i18n("Export…")
                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    }
                    QQC2.ToolButton {
                        icon.name: "edit-delete"
                        onClicked: removeTrip(root.tripGroupId)
                        text: i18n("Delete trip")
                        display: QQC2.AbstractButton.IconOnly
                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    }
                }
            }
        }
    }

    Accessible.name: headerLabel.text
    Accessible.onPressAction: root.clicked()
}
