/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef APPLICATIONCONTROLLER_H
#define APPLICATIONCONTROLLER_H

#include <QObject>

class DocumentManager;
class FavoriteLocationModel;
class HealthCertificateManager;
class LiveDataManager;
class PkPassManager;
class ReservationManager;
class TransferManager;
class TripGroupManager;

namespace KItinerary {
class ExtractorDocumentNode;
class File;
}

namespace KAndroidExtras {
class Intent;
}

class QNetworkAccessManager;

class ApplicationController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasClipboardContent READ hasClipboardContent NOTIFY clipboardContentChanged)
    Q_PROPERTY(QString version READ applicationVersion CONSTANT)
    Q_PROPERTY(QString extractorCapabilities READ extractorCapabilities CONSTANT)
    Q_PROPERTY(QVariant aboutData READ aboutData CONSTANT)
public:
    explicit ApplicationController(QObject *parent = nullptr);
    ~ApplicationController();

    void setReservationManager(ReservationManager *resMgr);
    void setPkPassManager(PkPassManager *pkPassMgr);
    void setDocumentManager(DocumentManager *docMgr);
    void setTransferManager(TransferManager *transferMgr);
    void setFavoriteLocationModel(FavoriteLocationModel *favLocModel);
    void setLiveDataManager(LiveDataManager *liveDataMgr);
    void setTripGroupManager(TripGroupManager *tripGroupMgr);
    void setHealthCertificateManager(HealthCertificateManager *healthCertMgr);

    // data import
    void importFromIntent(const KAndroidExtras::Intent &intent);
    Q_INVOKABLE void importFromClipboard();
    Q_INVOKABLE void importFromUrl(const QUrl &url);
    void importData(const QByteArray &data);

    Q_INVOKABLE void checkCalendar();

    static ApplicationController* instance();

    bool hasClipboardContent() const;

    void importBundle(const QUrl &url);
    void importBundle(const QByteArray &data);

    // data export
    Q_INVOKABLE void exportToFile(const QString &filePath);
    Q_INVOKABLE void exportTripToGpx(const QString &tripGroupId, const QString &filePath);

    // document attaching
    Q_INVOKABLE void addDocument(const QString &batchId, const QUrl &url);
    Q_INVOKABLE void removeDocument(const QString &batchId, const QString &docId);
    Q_INVOKABLE void openDocument(const QUrl &url);

    // about information
    QString applicationVersion() const;
    QString extractorCapabilities() const;
    QVariant aboutData() const;

Q_SIGNALS:
    void clipboardContentChanged();

    /** Human readable information message to be shown as passive notification. */
    void infoMessage(const QString &msg);

private:
    void importLocalFile(const QUrl &url);
    void importBundle(KItinerary::File *file);
    QVector<QString> importReservationOrHealthCertificate(const QByteArray &data, const QString &fileName = {});
    void importHealthCertificateRecursive(const KItinerary::ExtractorDocumentNode &node);
    void importPass(const QString &passId);

    static ApplicationController *s_instance;

    ReservationManager *m_resMgr = nullptr;
    PkPassManager *m_pkPassMgr = nullptr;
    DocumentManager *m_docMgr = nullptr;
    TransferManager *m_transferMgr = nullptr;
    FavoriteLocationModel *m_favLocModel = nullptr;
    LiveDataManager *m_liveDataMgr = nullptr;
    TripGroupManager *m_tripGroupMgr = nullptr;
    HealthCertificateManager *m_healthCertMgr = nullptr;
    QNetworkAccessManager *m_nam = nullptr;
};

#endif // APPLICATIONCONTROLLER_H
