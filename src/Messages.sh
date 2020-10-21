# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

#! /usr/bin/env bash
$XGETTEXT `find -name \*.cpp -o -name \*.qml -o -name \*.js` -o $podir/kde-itinerary.pot

