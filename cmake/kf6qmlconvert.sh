#!/bin/sh
# SPDX-FileCopyrightText: Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

# Kirigami.BasicListItem icon property changes
perl -0777 -p -i -e 's/(BasicListItem \{[^\}]*?)(icon): /\1icon.name: /sg' $1

# Kirigami actions.main property
perl -p -i -e 's/actions.main:/actions:/' $1

# Kirigami OverlaySheet signal changes
perl -p -i -e 's/onSheetOpenChanged:/onOpened:/' $1

# QtGraphicalEffects
perl -p -i -e 's/import QtGraphicalEffects 1\.\d+/import Qt5Compat.GraphicalEffects 6.0/' $1

# QtLocation
perl -p -i -e 's/import QtLocation 5.11/import QtLocation/' $1
