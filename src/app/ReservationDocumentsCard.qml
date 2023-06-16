/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.19 as Kirigami
import org.kde.kitinerary 1.0
import org.kde.itinerary 1.0
import "." as App

App.DocumentsCard {
    /// TimelineDelegateController
    property QtObject controller: null

    documentIds: controller.documentIds

    onAddDocument: (file) => { ApplicationController.addDocument(controller.batchId, file); }
    onRemoveDocument: (docId) => { ApplicationController.removeDocument(controller.batchId, docId); }
}
