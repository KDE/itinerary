/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <chrono>

namespace Constants {
constexpr std::chrono::seconds MaximumLayoverTime = std::chrono::hours(4);

constexpr std::chrono::seconds CurrentBatchLeadingMargin = std::chrono::hours(48);
constexpr std::chrono::seconds CurrentBatchTrailingMargin = std::chrono::hours(4);

}
