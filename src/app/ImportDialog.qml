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
import QtQuick.Dialogs 1.0

Item {
    function importReservation()
    {
        fileDialog.loadPass = false;
        fileDialog.visible = true;
    }

    function importPass()
    {
        fileDialog.loadPass = true;
        fileDialog.visible = true;
    }

    FileDialog {
        property bool loadPass: false
        id: fileDialog
        title: i18n("Please choose a file")
        folder: shortcuts.home
        onAccepted: {
            console.log(fileDialog.fileUrls);
            if (loadPass)
                _pkpassManager.importPass(fileDialog.fileUrl);
            else
                _reservationManager.importReservation(fileDialog.fileUrl);
        }
    }
}
