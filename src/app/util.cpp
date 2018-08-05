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

#include "util.h"

#include <KItinerary/JsonLdDocument>

#include <QDateTime>
#include <QTimeZone>
#include <QQuickItem>

using namespace KItinerary;

Util::Util(QObject* parent)
    : QObject(parent)
{
}

Util::~Util() = default;

QDateTime Util::dateTimeStripTimezone(const QVariant& obj, const QString& propertyName) const
{
    auto dt = JsonLdDocument::readProperty(obj, propertyName.toUtf8().constData()).value<QDateTime>();
    if (!dt.isValid()) {
        return {};
    }

    dt.setTimeSpec(Qt::LocalTime);
    return dt;
}

QVariant Util::setDateTimePreserveTimezone(const QVariant &obj, const QString& propertyName, QDateTime value) const
{
    QVariant o(obj);
    const auto oldDt = JsonLdDocument::readProperty(obj, propertyName.toUtf8().constData()).value<QDateTime>();
    if (oldDt.isValid()) {
        value.setTimeZone(oldDt.timeZone());
    }
    JsonLdDocument::writeProperty(o, propertyName.toUtf8().constData(), value);
    return o;
}

void Util::fixFormLayoutTouchTransparency(QObject *obj) const
{
    // HACK: this seems very wrong, but it actually makes touch screen scrolling work...
    QQuickItem *item = qobject_cast<QQuickItem*>(obj);
    if (item) {
        item->setAcceptedMouseButtons(Qt::AllButtons);
    }
}

#include "moc_util.cpp"
