// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitemmodels
import org.kde.prison.scanner as Prison
import org.kde.kitinerary
import org.kde.itinerary

/** Editor card for manually setting tickete or program membership barcodes. */
ColumnLayout {
    id: root

    /** The object to edit, either a Ticket or a ProgramMembership. */
    property var item

    /** Whether or not to allow selecting a ticket from PassManager. */
    property bool allowTicketSelection: true

    /** Card header title. */
    property alias title: header.title

    /** Apply changes to @p item, either a Ticket or a ProgramMembership. */
    function apply(item) {
        if (root.newToken === "" && ticketSelectionBox.currentIndex < 0)
            return;

        if (item.hasOwnProperty("ticketToken")) {
            if (ticketSelectionBox.currentIndex >= 0) {
                item.name = ticketSelectionBox.currentValue.name;
                item.issuedBy = ticketSelectionBox.currentValue.issuedBy;
                item.ticketNumber = ticketSelectionBox.currentValue.ticketNumber;
                item.ticketToken = ticketSelectionBox.currentValue.ticketToken;
                item.person = ticketSelectionBox.currentValue.person;
                item.validFrom = ticketSelectionBox.currentValue.validFrom;
                item.validUntil = ticketSelectionBox.currentValue.validUntil;
                item.identifier = ticketSelectionBox.currentValue.identifier;
            } else {
                item.ticketToken = root.newToken;
            }
        } else {
            item.token = root.newToken;
        }
    }

    // internal
    readonly property string token: root.item.hasOwnProperty("ticketToken") ? (root.item.ticketToken ?? "") : (root.item.token ?? "")
    property string newToken: ""

    spacing: 0

    FormCard.FormHeader {
        id: header
        title: i18nc("@title:group", "Ticket Barcode")
        visible: card.visible
    }

    FormCard.FormCard {
        id: card
        visible: root.token === ""

        FormCard.FormComboBoxDelegate {
            id: ticketSelectionBox
            text: i18n("Select ticket")
            icon.name: "bookmarks"
            visible: ticketModel.count > 0 && root.allowTicketSelection
            model: KSortFilterProxyModel {
                id: ticketModel
                filterRowCallback: function(source_row) {
                    const type = sourceModel.data(sourceModel.index(source_row, 0), PassManager.PassTypeRole);
                    const validUntil = sourceModel.data(sourceModel.index(source_row, 0), PassManager.ValidUntilRole);
                    return type == PassManager.Ticket && (isNaN(validUntil) || validUntil > new Date());
                }
                sourceModel: PassManager
            }
            textRole: "name"
            valueRole: "pass"
            onCurrentIndexChanged: {
                if (ticketSelectionBox.currentIndex >= 0)
                    root.newToken = "";
            }
        }

        FormCard.FormDelegateSeparator { visible: ticketSelectionBox.visible }

        FormCard.FormButtonDelegate {
            text: i18n("Scan barcode…")
            icon.name: "view-barcode-qr"
            onClicked: {
                const pageComponent = Qt.createComponent("org.kde.itinerary", "BarcodeScannerPage");
                let page = pageComponent.createObject();
                page.supportedFormats = Prison.Format.Aztec | Prison.Format.DataMatrix | Prison.Format.PDF417 | Prison.Format.QRCode |
                                        Prison.Format.Code39 | Prison.Format.Code128 | Prison.Format.EAN13;
                page.barcodeDetected.connect((result) => {
                    if (result.hasText) {
                        switch (result.format) {
                            case Prison.Format.Aztec: root.newToken = "azteccode:" + result.text; break;
                            case Prison.Format.Code39: root.newToken = "code39:" + result.text; break;
                            case Prison.Format.Code128: root.newToken = "barcode128:" + result.text; break;
                            case Prison.Format.DataMatrix: root.newToken = "datamatrix:" + result.text; break;
                            case Prison.Format.EAN13: root.newToken = "ean13:" + result.text; break;
                            case Prison.Format.PDF417: root.newToken = "pdf417:" + result.text; break;
                            case Prison.Format.QRCode: root.newToken = "qrcode:" + result.text; break;
                        }
                    } else if (result.hasBinaryData) {
                        switch (result.format) {
                            case Prison.Format.Aztec: root.newToken = "aztecbin:" + Util.toBase64(result.binaryData); break;
                            case Prison.Format.PDF417: root.newToken = "pdf417bin:" + Util.toBase64(result.binaryData); break;
                            case Prison.Format.QRCode: root.newToken = "qrcodebin:" + Util.toBase64(result.binaryData); break;
                        }
                    }

                    if (root.newToken !== "") {
                        ticketSelectionBox.currentIndex = -1;
                        applicationWindow().pageStack.pop();
                    }
                });
                applicationWindow().pageStack.push(page);
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: i18n("Paste")
            icon.name: "edit-paste"
            enabled: Clipboard.hasText || Clipboard.hasBinaryData
            onClicked: {
                if (Clipboard.hasText) {
                    root.newToken = "qrcode:" + Clipboard.text;
                    ticketSelectionBox.currentIndex = -1;
                } else if (Clipboard.hasBinaryData) {
                    root.newToken = "qrcodebin:" + Util.toBase64(Clipboard.binaryData);
                    ticketSelectionBox.currentIndex = -1;
                }
            }
        }
    }
}
