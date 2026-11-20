// SPDX-FileCopyrightText: © 2026 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

// see https://www.w3.org/TR/WCAG20/#relativeluminancedef
function relativeLuminance(c)
{
    const r = (c.r <= 0.03928) ? (c.r / 12.92) : Math.pow(((c.r + 0.055) / 1.055), 2.4);
    const g = (c.g <= 0.03928) ? (c.g / 12.92) : Math.pow(((c.g + 0.055) / 1.055), 2.4);
    const b = (c.b <= 0.03928) ? (c.b / 12.92) : Math.pow(((c.b + 0.055) / 1.055), 2.4);
    return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}

// see https://www.w3.org/TR/WCAG20/#contrast-ratiodef
function relativeContrast(color1, color2)
{
    const l1 = relativeLuminance(color1) + 0.05;
    const l2 = relativeLuminance(color2) + 0.05;
    return l1 > l2 ? (l1 / l2) : (l2 / l1);
}
