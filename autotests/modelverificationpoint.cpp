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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "modelverificationpoint.h"

#include <KItinerary/JsonLdDocument>

#include <QAbstractItemModel>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

ModelVerificationPoint::ModelVerificationPoint(const QString &refFile)
    : m_refFile(refFile)
{
}

ModelVerificationPoint::~ModelVerificationPoint() = default;

void ModelVerificationPoint::setRoleFilter(std::vector<int> &&filter)
{
    m_roleFilter = std::move(filter);
}

static QJsonValue variantToJson(const QVariant &v)
{
    switch (v.type()) {
        case QVariant::String:
            return v.toString();
        case QVariant::StringList:
        {
            const auto l = v.toStringList();
            if (l.isEmpty()) {
                return {};
            }
            QJsonArray a;
            std::copy(l.begin(), l.end(), std::back_inserter(a));
            return a;
        }
        case QVariant::Bool:
            return v.toBool();
        case QVariant::Int:
            return v.toInt();
        default:
            break;
    }

    if (QMetaType::metaObjectForType(v.userType())) {
        return KItinerary::JsonLdDocument::toJson(v);
    } else if (v.userType() == qMetaTypeId<QVector<QVariant>>()) {
        return KItinerary::JsonLdDocument::toJson(v.value<QVector<QVariant>>());
    }

    return {};
}

bool ModelVerificationPoint::verify(QAbstractItemModel *model) const
{
    // serialize model state
    QJsonArray array;
    const auto roleNames = model->roleNames();
    for (int i = 0; i < model->rowCount(); ++i) {
        const auto idx = model->index(i, 0);
        QJsonObject obj;
        for (auto it = roleNames.begin(); it != roleNames.end(); ++it) {
            if (std::find(m_roleFilter.begin(), m_roleFilter.end(), it.key()) != m_roleFilter.end()) {
                continue;
            }

            const auto v = variantToJson(idx.data(it.key()));
            if (!v.isNull() && !(v.isArray() && v.toArray().isEmpty())) {
                obj.insert(QString::fromUtf8(it.value()), v);
            }
        }
        array.push_back(obj);
    }

    // reference data
    QFile file(m_refFile);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << file.fileName() << file.errorString();
        return false;
    }
    const auto doc = QJsonDocument::fromJson(file.readAll());
    const auto refArray = doc.array();

    // compare
    if (array != refArray) {
        QFile failFile(m_refFile + QLatin1String(".fail"));
        failFile.open(QFile::WriteOnly);
        failFile.write(QJsonDocument(array).toJson());
        failFile.close();

        QProcess proc;
        proc.setProcessChannelMode(QProcess::ForwardedChannels);
        proc.start(QStringLiteral("diff"), {QStringLiteral("-u"), m_refFile, failFile.fileName()});
        proc.waitForFinished();
        return false;
    }

    return true;
}
