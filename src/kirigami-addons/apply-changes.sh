#!/bin/bash
# SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: CC0-1.0

# apply changes on top of the upstream copy of kirigami-addons
perl -p -i -e 's/org.kde.kirigamiaddons.dateandtime/org.kde.itinerary.kirigamiaddons.dateandtime/' dateandtime/CMakeLists.txt
perl -p -i -e 's/(?<!-)dateandtimeplugin/itinerary-dateandtimeplugin/g' dateandtime/CMakeLists.txt
perl -p -i -e 's/(^target_compile_definitions.*TRANSLATION_DOMAIN.*$)/#\1/' dateandtime/CMakeLists.txt

for i in `find -name *.qml`; do
    perl -p -i -e 's/import org.kde.kirigamiaddons.dateandtime 0.1/import org.kde.itinerary.kirigamiaddons.dateandtime 0.1/' $i
done
