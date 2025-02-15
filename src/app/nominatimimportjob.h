/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef NOMINATIMIMPORTJOB_H
#define NOMINATIMIMPORTJOB_H

#include <KOSM/Datatypes>

#include <QObject>
#include <QVariant>

class QJsonObject;
class QNetworkAccessManager;
class QNetworkReply;

/** Import restaurants/hotels/etc from OSM data. */
class NominatimImportJob : public QObject
{
    Q_OBJECT
public:
    explicit NominatimImportJob(OSM::Type type, OSM::Id id, QNetworkAccessManager *nam, QObject *parent = nullptr);
    ~NominatimImportJob();

    /** Retrieved element. */
    [[nodiscard]] QVariant result() const;
    [[nodiscard]] QString errorMessage() const;

Q_SIGNALS:
    /** Emitted when the job finished, regardless of success.
     *  Deletion of the job afterwards is consumer responsibility.
     */
    void finished();

private:
    void handleReply(QNetworkReply *reply);

    [[nodiscard]] static QVariant convertElement(const QJsonObject &feature);

    OSM::Type m_type = OSM::Type::Null;
    OSM::Id m_id = {};

    QVariant m_result;
    QString m_errorMsg;
};

#endif
