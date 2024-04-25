/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSMIMPORTJOB_H
#define OSMIMPORTJOB_H

#include <KOSM/Datatypes>

#include <QObject>
#include <QVariant>

namespace OSM {
class Element;
}

class QNetworkAccessManager;
class QNetworkReply;

/** Import restaurants/hotels/etc from OSM data. */
class OsmImportJob : public QObject
{
    Q_OBJECT
public:
    explicit OsmImportJob(OSM::Type type, OSM::Id id, QNetworkAccessManager *nam, QObject *parent = nullptr);
    ~OsmImportJob();

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

    [[nodiscard]] static QVariant convertElement(OSM::Element e);

    OSM::Type m_type = OSM::Type::Null;
    OSM::Id m_id = {};

    QVariant m_result;
    QString m_errorMsg;
};

#endif // OSMIMPORTJOB_H
