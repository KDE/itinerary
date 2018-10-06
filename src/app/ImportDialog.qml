/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.0
import Qt.labs.platform 1.0 as Platform

Item {
    function importReservation()
    {
        fileDialog.visible = true;
    }

    Platform.FileDialog {
        id: fileDialog
        title: i18n("Import Reservation")
        nameFilters: [i18n("PkPass files (*.pkpass)"), i18n("JSON files (*.json)"), i18n("PDF files (*.pdf)")]
        folder: Platform.StandardPaths.writableLocation(Platform.StandardPaths.DocumentsLocation)
        onAccepted: {
            _appController.importFromUrl(fileDialog.file);
        }
    }
}
