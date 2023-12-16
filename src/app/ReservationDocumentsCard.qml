/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kitinerary
import org.kde.itinerary
import "." as App

App.DocumentsCard {
    /// TimelineDelegateController
    property QtObject controller: null

    documentIds: controller.documentIds
    title: i18n("Documents and Tickets")

    onAddDocument: (file) => { ApplicationController.addDocumentToReservation(controller.batchId, file); }
    onRemoveDocument: (docId) => { ApplicationController.removeDocumentFromReservation(controller.batchId, docId); }
}
