/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MODELVERIFICATIONPOINT_H
#define MODELVERIFICATIONPOINT_H

#include <QString>

#include <vector>

class QAbstractItemModel;
class QJsonValue;
class QVariant;

/** Compares a model state serialized to JSON with a current model state. */
class ModelVerificationPoint
{
public:
    explicit ModelVerificationPoint(const QString &refFile);
    ~ModelVerificationPoint();

    void setRoleFilter(std::vector<int> &&filter);
    void setJsonPropertyFilter(std::vector<QString> &&filter);

    bool verify(QAbstractItemModel *model) const;

private:
    QJsonValue variantToJson(const QVariant &v) const;

    QString m_refFile;
    std::vector<int> m_roleFilter;
    std::vector<QString> m_jsonPropertyFilter;
};

#endif // MODELVERIFICATIONPOINT_H
