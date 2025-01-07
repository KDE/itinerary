// SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kpublictransport as KPublicTransport
import "../src/app"

Kirigami.Page {
    width: 600
    height: 800

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        TransportNameControl {
            lineName: "Test"
            iconName: "car"
            journeySectionMode: KPublicTransport.JourneySection.PublicTransport
        }

        // light on dark
        RowLayout {
            TransportNameControl {
                line: ({
                    mode: KPublicTransport.Line.Train,
                    name: "U1",
                    color: "#65BD00",
                    textColor: "white"
                })
            }
            TransportNameControl {
                line: ({
                    mode: KPublicTransport.Line.Train,
                    name: "U1",
                    color: "#65BD00"
                })
            }
        }

        // dark on light
        RowLayout {
            TransportNameControl {
                journeySection: ({
                    mode: KPublicTransport.JourneySection.PublicTransport,
                    route: {
                        line: {
                            mode: KPublicTransport.Line.Metro,
                            name: "U4",
                            color: "#FFD900",
                            textColor: "black"
                        }
                    }
                })
            }
            TransportNameControl {
                journeySection: ({
                    mode: KPublicTransport.JourneySection.PublicTransport,
                    route: {
                        line: {
                            mode: KPublicTransport.Line.Metro,
                            name: "U4",
                            color: "#FFD900"
                        }
                    }
                })
            }
        }
        TransportNameControl {
            journeySection: ({
                mode: KPublicTransport.JourneySection.Walking
            })
        }

        RowLayout {
            TransportNameControl {
                journeySection: ({
                    mode: KPublicTransport.JourneySection.IndividualTransport,
                    individualTransport: { mode: KPublicTransport.IndividualTransport.Bike }
                })
            }
            TransportNameControl {
                journeySection: ({
                    mode: KPublicTransport.JourneySection.PublicTransport,
                    route: {
                        line: {
                            mode: KPublicTransport.Line.Bus,
                            name: "123"
                        }
                    }
                })
                Layout.fillWidth: true
            }
            TransportNameControl {
                journeySection: ({
                    mode: KPublicTransport.JourneySection.Transfer
                })
            }
            TransportNameControl {
                journeySection: ({
                    mode: KPublicTransport.JourneySection.PublicTransport,
                    route: {
                        line: {
                            mode: KPublicTransport.Line.LongDistanceTrain,
                            name: "TGV 9876"
                        }
                    }
                })
                Layout.fillWidth: true
            }
            TransportNameControl {
                journeySection: ({
                    mode: KPublicTransport.JourneySection.RentedVehicle,
                    rentalVehicle: { type: KPublicTransport.RentalVehicle.ElectricKickScooter }
                })
            }
        }
    }
}
