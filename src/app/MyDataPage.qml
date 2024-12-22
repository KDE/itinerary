// SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtCore as QtCore
import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import Qt.labs.qmlmodels as Models
import org.kde.kitemmodels
import org.kde.i18n.localeData
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.itinerary

FormCard.FormCardPage {
    id: root

    title: i18nc("@title", "My Data")

    // Passes and Programs
    FormCard.FormHeader {
        title: i18nc("@title", "Passes and Programs")
    }

    FormCard.FormCard {
        FormCard.FormPlaceholderMessageDelegate {
            visible: passRepeater.count === 0
            icon.name: "wallet-open"
            text: i18n("No valid bonus or discount program cards or flat rate passes found.")
        }

        Repeater {
            id: passRepeater

            model: KSortFilterProxyModel {
                sourceModel: PassManager
                filterRoleName: "state"
                filterRegularExpression: /valid/
            }

            delegate: PassDelegate {}
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Manage your Passes")
            onClicked: QQC2.ApplicationWindow.window.pageStack.push(Qt.createComponent("org.kde.itinerary", "PassPage"))
        }
    }

    // Health Certificates
    FormCard.FormHeader {
        title: i18nc("@title", "Health Certificates")
        visible: ApplicationController.hasHealthCertificateSupport && ApplicationController.healthCertificateManager.rowCount() > 0
    }

    FormCard.FormCard {
        visible: ApplicationController.hasHealthCertificateSupport && ApplicationController.healthCertificateManager.rowCount() > 0

        FormCard.FormPlaceholderMessageDelegate {
            visible: healthCertificateRepeater.count === 0
            icon.name: "cross-shape"
            text: i18n("No valid health certificate found.")
        }

        Repeater {
            id: healthCertificateRepeater

            model: KSortFilterProxyModel {
                sourceModel: ApplicationController.healthCertificateManager
                filterRowCallback: function(source_row, source_parent) {
                    const isValid = sourceModel.data(sourceModel.index(source_row, 0, source_parent), HealthCertificateManager.IsValidRole);
                    return isValid;
                };
            }

            delegate: FormCard.FormButtonDelegate {
                required property string name
                required property string certificateName
                required property var certificate

                icon.name: "cross-shape"
                text: certificateName
                description: name

                onClicked: {
                    QQC2.ApplicationWindow.window.pageStack.push(Qt.resolvedUrl("HealthCertificatePage.qml"), {
                        initialCertificate: certificate,
                    })
                }
            }
        }
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Manage your health certificates")
            onClicked: QQC2.ApplicationWindow.window.pageStack.push(Qt.createComponent("org.kde.itinerary", "HealthCertificatePage"))
        }

    }

    // Saved location
    FormCard.FormHeader {
        title: i18nc("@title:group", "Saved Locations")
    }

    FormCard.FormCard {
        CountryComboBoxDelegate {
            text: i18n("Home Country")
            model: Country.allCountries.map(c => c.alpha2).sort((lhs, rhs) => {
                return Country.fromAlpha2(lhs).name.localeCompare(Country.fromAlpha2(rhs).name);
            })
            initialCountry: Settings.homeCountryIsoCode
            onActivated: Settings.homeCountryIsoCode = currentValue
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: i18n("Favorite Locations")
            icon.name: "go-home-symbolic"
            onClicked: applicationWindow().pageStack.layers.push(Qt.createComponent("org.kde.itinerary", "FavoriteLocationPage"))
        }
    }

    FormCard.FormHeader {
        title: i18n("Statistics")
    }

    FormCard.FormGridContainer {
        Layout.fillWidth: true

        StatisticsModel {
            id: model
            reservationManager: ReservationManager
            tripGroupManager: TripGroupManager
            transferManager: TransferManager
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

        infoCards: [
            FormCard.FormGridContainer.InfoCard {
                title: model.totalCount.label
                subtitle: model.totalCount.value
            },
            FormCard.FormGridContainer.InfoCard {
                title: model.totalDistance.label
                subtitle: model.totalDistance.value
            },
            FormCard.FormGridContainer.InfoCard {
                title: model.totalNights.label
                subtitle: model.totalNights.value
            },
            FormCard.FormGridContainer.InfoCard {
                title: model.totalCO2.label
                subtitle: model.totalCO2.value
            }
        ]
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.smallSpacing

        FormCard.FormButtonDelegate {
            icon.name: 'view-statistics-symbolic'
            text: i18nc("@action:button", "See all Statistics")
            onClicked: QQC2.ApplicationWindow.window.pageStack.push(Qt.createComponent("org.kde.itinerary", "StatisticsPage"), {
                reservationManager: ReservationManager,
                tripGroupManager: TripGroupManager,
                transferManager: TransferManager,
            })
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Data Portability")
    }

    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Import")
            icon.name: 'document-import-symbolic'
            onClicked: importFileDialog.open();
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Export")
            icon.name: "document-export-symbolic"
            onClicked: {
                const today = new Date();
                exportDialog.currentFile = today.toISOString().substr(0, 10) + "-kde-itinerary-backup.itinerary"
                exportDialog.open();
            }

            FileDialog {
                id: exportDialog
                fileMode: FileDialog.SaveFile
                title: i18n("Export Itinerary Data")
                currentFolder: QtCore.StandardPaths.writableLocation(QtCore.StandardPaths.DocumentsLocation)
                nameFilters: [i18n("KDE Itinerary files (*.itinerary)")]
                onAccepted: ApplicationController.exportToFile(selectedFile)
            }
        }
    }
}
