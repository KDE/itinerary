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
class PassManager;
class PkPassManager;
class ReservationManager;
class TransferManager;
class TripGroupManager;

namespace KItinerary {
class ExtractorDocumentNode;
class File;
}

namespace KMime { class Message; }

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
    ~ApplicationController() override;

    void requestOpenPage(const QString &page);

    void setReservationManager(ReservationManager *resMgr);
    void setPkPassManager(PkPassManager *pkPassMgr);
    void setDocumentManager(DocumentManager *docMgr);
    void setTransferManager(TransferManager *transferMgr);
    void setFavoriteLocationModel(FavoriteLocationModel *favLocModel);
    void setLiveDataManager(LiveDataManager *liveDataMgr);
    void setTripGroupManager(TripGroupManager *tripGroupMgr);
    void setPassManager(PassManager *passMgr);
    void setHealthCertificateManager(HealthCertificateManager *healthCertMgr);

    // data import
    void importFromIntent(const KAndroidExtras::Intent &intent);
    Q_INVOKABLE void importFromClipboard();
    Q_INVOKABLE void importFromUrl(const QUrl &url);
    void importData(const QByteArray &data, const QString &fileName = {});

    Q_INVOKABLE void checkCalendar();

    static ApplicationController* instance();

    // data export
    Q_INVOKABLE void exportToFile(const QUrl &url);
    Q_INVOKABLE void exportTripToGpx(const QString &tripGroupId, const QUrl &url);

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

    void openPageRequested(const QString &page);

private:
    bool hasClipboardContent() const;

    void importLocalFile(const QUrl &url);
    bool importBundle(const QUrl &url);
    bool importBundle(const QByteArray &data);
    bool importBundle(KItinerary::File *file);
    bool importHealthCertificateRecursive(const KItinerary::ExtractorDocumentNode &node);
    void importPass(const QString &passId);
    void importMimeMessage(KMime::Message *msg);
    void importNode(const KItinerary::ExtractorDocumentNode &node);
    bool importGenericPkPass(const KItinerary::ExtractorDocumentNode &node);

    static ApplicationController *s_instance;

    ReservationManager *m_resMgr = nullptr;
    PkPassManager *m_pkPassMgr = nullptr;
    DocumentManager *m_docMgr = nullptr;
    TransferManager *m_transferMgr = nullptr;
    FavoriteLocationModel *m_favLocModel = nullptr;
    LiveDataManager *m_liveDataMgr = nullptr;
    TripGroupManager *m_tripGroupMgr = nullptr;
    PassManager *m_passMgr = nullptr;
    HealthCertificateManager *m_healthCertMgr = nullptr;
    QNetworkAccessManager *m_nam = nullptr;

    bool m_importLock = false;
};

#endif // APPLICATIONCONTROLLER_H
