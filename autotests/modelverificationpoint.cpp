/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

void ModelVerificationPoint::setJsonPropertyFilter(std::vector<QString> &&filter)
{
    m_jsonPropertyFilter = std::move(filter);
}

QJsonValue ModelVerificationPoint::variantToJson(const QVariant &v) const
{
    switch (v.userType()) {
        case QMetaType::QString:
            return v.toString();
        case QMetaType::QStringList:
        {
            const auto l = v.toStringList();
            if (l.isEmpty()) {
                return {};
            }
            QJsonArray a;
            std::copy(l.begin(), l.end(), std::back_inserter(a));
            return a;
        }
        case QMetaType::Bool:
            return v.toBool();
        case QMetaType::Int:
            return v.toInt();
        default:
            break;
    }

    if (QMetaType(v.userType()).metaObject()) {
        auto obj = KItinerary::JsonLdDocument::toJson(v);
        for (const auto &filter : m_jsonPropertyFilter) {
            obj.remove(filter);
        }
        return obj;
    } else if (v.userType() == qMetaTypeId<QList<QVariant>>()) {
      return KItinerary::JsonLdDocument::toJson(v.value<QList<QVariant>>());
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
        QFile failFile(m_refFile + QLatin1StringView(".fail"));
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
