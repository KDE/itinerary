/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef APPLICATIONCONTROLLER_H
#define APPLICATIONCONTROLLER_H

#include <QObject>
#include <QVariantMap>

#include <memory>

class DocumentManager;
class FavoriteLocationModel;
class HealthCertificateManager;
class ImportController;
class LiveDataManager;
class PassManager;
class PkPassManager;
class ReservationManager;
class TransferManager;
class TripGroupManager;

namespace KItinerary
{
class ExtractorDocumentNode;
class File;
}

namespace KAndroidExtras
{
class Intent;
}

class QNetworkAccessManager;
class QTemporaryDir;

class ApplicationController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version READ applicationVersion CONSTANT)
    Q_PROPERTY(QString extractorCapabilities READ extractorCapabilities CONSTANT)
    Q_PROPERTY(QVariant aboutData READ aboutData CONSTANT)
    Q_PROPERTY(QString userAgent READ userAgent CONSTANT)

    /** The current trip group context, ie. the currently looked at trip group.
     *  Not to be confused with the current trip group, ie. the one happening now.
     */
    Q_PROPERTY(QString contextTripGroupId MEMBER m_contextTripGroupId WRITE setContextTripGroupId NOTIFY contextTripGroupIdChanged)

    Q_PROPERTY(bool hasHealthCertificateSupport READ hasHealthCertificateSupport CONSTANT)
    Q_PROPERTY(HealthCertificateManager *healthCertificateManager READ healthCertificateManager CONSTANT)
public:
    explicit ApplicationController(QObject *parent = nullptr);
    ~ApplicationController() override;

    void setNetworkAccessManagerFactory(const std::function<QNetworkAccessManager *()> &namFactory);

    void requestOpenPage(const QString &page, const QVariantMap &properties = {});

    void setReservationManager(ReservationManager *resMgr);
    void setPkPassManager(PkPassManager *pkPassMgr);
    void setDocumentManager(DocumentManager *docMgr);
    void setTransferManager(TransferManager *transferMgr);
    void setFavoriteLocationModel(FavoriteLocationModel *favLocModel);
    void setLiveDataManager(LiveDataManager *liveDataMgr);
    void setTripGroupManager(TripGroupManager *tripGroupMgr);
    void setPassManager(PassManager *passMgr);

    // page navigation intents
    void handleIntent(const KAndroidExtras::Intent &intent);
    void handleGeoUrl(const QUrl &url);

    // data import
    Q_INVOKABLE void commitImport(ImportController *importController);
    /** Add a newly created reservation and add it to a trip group. */
    Q_INVOKABLE [[nodiscard]] QString addNewReservation(const QVariant &res, const QString &tgId);

    static ApplicationController *instance();

    // data export
    Q_INVOKABLE void exportToFile(const QUrl &url);
    Q_INVOKABLE void exportTripToFile(const QString &tripGroupId, const QUrl &url);
    Q_INVOKABLE void exportTripToKDEConnect(const QString &tripGroupId, const QString &deviceId);
    Q_INVOKABLE void exportTripToGpx(const QString &tripGroupId, const QUrl &url);
    Q_INVOKABLE void exportBatchToFile(const QString &batchId, const QUrl &url);
    Q_INVOKABLE void exportBatchToKDEConnect(const QString &batchId, const QString &deviceId);

    // document attaching
    Q_INVOKABLE void addDocumentToReservation(const QString &batchId, const QUrl &url);
    Q_INVOKABLE void removeDocumentFromReservation(const QString &batchId, const QString &docId);
    Q_INVOKABLE void addDocumentToPass(const QString &passId, const QUrl &url);
    Q_INVOKABLE void removeDocumentFromPass(const QString &passId, const QString &docId);
    Q_INVOKABLE void openDocument(const QUrl &url);

    // about information
    QString applicationVersion() const;
    QString extractorCapabilities() const;
    QVariant aboutData() const;
    static QString userAgent();

    // health certificate manager
    bool hasHealthCertificateSupport() const;
    HealthCertificateManager *healthCertificateManager() const;

    void setContextTripGroupId(const QString &contextTripGroupId);

Q_SIGNALS:
    /** Human readable information message to be shown as passive notification. */
    void infoMessage(const QString &msg);

    void openPageRequested(const QString &page, const QVariantMap &properties);

    /** Edit and add a new reservation. */
    void editNewHotelReservation(const QVariant &res);
    void editNewRestaurantReservation(const QVariant &res);
    void editNewEventReservation(const QVariant &res);

    void contextTripGroupIdChanged();

    /** Indicates a backup restore changed application settings. */
    void reloadSettings();

private:
    bool importBundle(KItinerary::File *file);
    void pkPassUpdated(const QString &passId);

    QString addDocumentFromFile(const QUrl &url);

    bool exportTripToFile(const QString &tripGroupId, const QString &fileName);
    bool exportBatchToFile(const QString &batchId, const QString &fileName);

    static ApplicationController *s_instance;

    ReservationManager *m_resMgr = nullptr;
    PkPassManager *m_pkPassMgr = nullptr;
    DocumentManager *m_docMgr = nullptr;
    TransferManager *m_transferMgr = nullptr;
    FavoriteLocationModel *m_favLocModel = nullptr;
    LiveDataManager *m_liveDataMgr = nullptr;
    TripGroupManager *m_tripGroupMgr = nullptr;
    PassManager *m_passMgr = nullptr;
    mutable HealthCertificateManager *m_healthCertMgr = nullptr;
    std::function<QNetworkAccessManager *()> m_namFactory;

    QString m_contextTripGroupId;

    std::unique_ptr<QTemporaryDir> m_tempDir;
    bool m_importLock = false;
};

#endif // APPLICATIONCONTROLLER_H
