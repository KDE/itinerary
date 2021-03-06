# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(pkpassviewer pkpassviewer.cpp pkpassviewer.qrc)
target_link_libraries(pkpassviewer itinerary)

add_executable(journeyquerytest journeyquerytest.cpp journeyquerytest.qrc)
target_link_libraries(journeyquerytest itinerary)

add_executable(vehiclelayoutviewer vehiclelayoutviewer.cpp vehiclelayoutviewer.qrc)
target_link_libraries(vehiclelayoutviewer itinerary)
