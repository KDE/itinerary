/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "applicationcontroller.h"
#include "bundle-constants.h"
#include "documentmanager.h"
#include "downloadjob.h"
#include "favoritelocationmodel.h"
#include "filehelper.h"
#include "genericpkpass.h"
#include "gpxexport.h"
#include "healthcertificatemanager.h"
#include "importcontroller.h"
#include "importexport.h"
#include "kdeconnect.h"
#include "livedatamanager.h"
#include "logging.h"
#include "passmanager.h"
#include "pkpassmanager.h"
#include "reservationmanager.h"
#include "transfermanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"
#include <itinerary_version_detailed.h>

#include <KItinerary/CreativeWork>
#include <KItinerary/DocumentUtil>
#include <KItinerary/Event>
#include <KItinerary/ExtractorCapabilities>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/File>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Place>
#include <KItinerary/Reservation>

#include <KPkPass/Pass>

#include <KAboutData>
#include <KLocalizedString>

#include <QBuffer>
#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QMimeDatabase>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QScopedValueRollback>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QUuid>
#include <QUrl>
#include <QUrlQuery>

#ifdef Q_OS_ANDROID
#include "android/itineraryactivity.h"

#include "kandroidextras/activity.h"
#include "kandroidextras/contentresolver.h"
#include "kandroidextras/intent.h"
#include "kandroidextras/jniarray.h"
#include "kandroidextras/jnisignature.h"
#include "kandroidextras/jnitypes.h"
#include "kandroidextras/uri.h"
#endif

using namespace Qt::Literals::StringLiterals;
using namespace KItinerary;

ApplicationController* ApplicationController::s_instance = nullptr;

ApplicationController::ApplicationController(QObject* parent)
    : QObject(parent)
{
    s_instance = this;
}

ApplicationController::~ApplicationController()
{
    s_instance = nullptr;
}

void ApplicationController::setNetworkAccessManagerFactory(const std::function<QNetworkAccessManager*()> &namFactory)
{
    m_namFactory = namFactory;
}

ApplicationController* ApplicationController::instance()
{
    return s_instance;
}

void ApplicationController::requestOpenPage(const QString &page)
{
    Q_EMIT openPageRequested(page);
}

void ApplicationController::setReservationManager(ReservationManager* resMgr)
{
    m_resMgr = resMgr;
}

void ApplicationController::setPkPassManager(PkPassManager* pkPassMgr)
{
    m_pkPassMgr = pkPassMgr;
    connect(m_pkPassMgr, &PkPassManager::passUpdated, this, &ApplicationController::importPass);
}

void ApplicationController::setDocumentManager(DocumentManager* docMgr)
{
    m_docMgr = docMgr;
}

void ApplicationController::setTransferManager(TransferManager* transferMgr)
{
    m_transferMgr = transferMgr;
}

void ApplicationController::setFavoriteLocationModel(FavoriteLocationModel *favLocModel)
{
    m_favLocModel = favLocModel;
}

void ApplicationController::setLiveDataManager(LiveDataManager *liveDataMgr)
{
    m_liveDataMgr = liveDataMgr;
}

void ApplicationController::setTripGroupManager(TripGroupManager *tripGroupMgr)
{
    m_tripGroupMgr = tripGroupMgr;
}

void ApplicationController::setPassManager(PassManager *passMgr)
{
    m_passMgr = passMgr;
}

void ApplicationController::handleIntent(const KAndroidExtras::Intent &intent)
{
#ifdef Q_OS_ANDROID
    using namespace KAndroidExtras;
    const auto action = intent.getAction();

    // opening a URL, can be something to import or a shortcut path
    if (action == Intent::ACTION_VIEW) {
        const QUrl url = intent.getData();
        if (url.scheme() == "page"_L1) {
            qCDebug(Log) << url;
            requestOpenPage(url.path().mid(1));
        }
    }
#else
    Q_UNUSED(intent)
#endif
}

void ApplicationController::commitImport(ImportController *importController)
{
    qCDebug(Log);

    int reservationCount = 0;
    int passCount = 0;
    int healthCertCount = 0;

    for (const auto &elem : importController->elements()) {
        if (!elem.selected) {
            continue;
        }

        QVariantList docIds;
        switch (elem.type) {
            case ImportElement::Reservation:
                m_resMgr->addReservation(elem.updateData.isNull() ? elem.data : elem.updateData, elem.id);
                docIds = DocumentUtil::documentIds(elem.data);
                for (const auto &r : elem.batch) {
                    m_resMgr->addReservation(r);
                    docIds += DocumentUtil::documentIds(r);
                }

                if (elem.bundleIdx >= 0 && !elem.id.isEmpty()) {
                    const auto bundle = importController->bundles()[elem.bundleIdx].data.get();

                    auto t = Transfer::fromJson(QJsonDocument::fromJson(bundle->customData(BUNDLE_TRANSFER_DOMAIN, Transfer::identifier(elem.id, Transfer::Before))).object());
                    m_transferMgr->importTransfer(t);
                    t = Transfer::fromJson(QJsonDocument::fromJson(bundle->customData(BUNDLE_TRANSFER_DOMAIN, Transfer::identifier(elem.id, Transfer::After))).object());
                    m_transferMgr->importTransfer(t);

                    const auto obj = QJsonDocument::fromJson(bundle->customData(BUNDLE_LIVE_DATA_DOMAIN, elem.id)).object();
                    if (!obj.isEmpty()) {
                        auto ld = LiveData::fromJson(obj);
                        ld.store(elem.id);
                        m_liveDataMgr->importData(elem.id, std::move(ld));
                    }
                }

                ++reservationCount;
                break;
            case ImportElement::Pass:
                docIds = DocumentUtil::documentIds(elem.data);
                m_passMgr->import(elem.data, elem.id);
                ++passCount;
                break;
            case ImportElement::HealthCertificate:
                m_healthCertMgr->importCertificate(HealthCertificateManager::certificateRawData(elem.data));
                ++healthCertCount;
                break;
            case ImportElement::Template:
                if (JsonLd::isA<LodgingBusiness>(elem.data)) { // TODO can't we do the reservation promotion in ImportController and share the model representation with reservations?
                    LodgingReservation res;
                    res.setReservationFor(elem.data);
                    res.setPotentialAction(elem.data.value<LodgingBusiness>().potentialAction());
                    Q_EMIT editNewHotelReservation(res);
                } else if (JsonLd::isA<FoodEstablishment>(elem.data) || JsonLd::isA<LocalBusiness>(elem.data)) {
                    // LocalBusiness is frequently used for restaurants in website annotations
                    FoodEstablishmentReservation res;
                    res.setReservationFor(elem.data);
                    res.setPotentialAction(JsonLd::convert<Organization>(elem.data).potentialAction());
                    Q_EMIT editNewRestaurantReservation(res);
                } else if (JsonLd::isA<EventReservation>(elem.data)) {
                    Q_EMIT editNewEventReservation(elem.data);
                }
                break;
            case ImportElement::Backup:
                importBundle(importController->bundles()[elem.bundleIdx].data.get());
                break;
        }

        for (const auto &docId : docIds) {
            if (const QUrl pkPassId(docId.toString()); pkPassId.scheme() == "pkpass"_L1) {
                const auto it = importController->pkPasses().find(docId.toString());
                if (it != importController->pkPasses().end()) {
                    QScopedValueRollback importLocker(m_importLock, true);
                    m_pkPassMgr->importPassFromData((*it).second.data);
                    importController->pkPasses().erase(it);
                }
            } else {
                const auto it = importController->documents().find(docId.toString());
                if (it != importController->documents().end()) {
                    m_docMgr->addDocument((*it).first, (*it).second.metaData, (*it).second.data);
                    importController->documents().erase(it);
                }
            }
        }
    }

    importController->clearSelected();

    if (reservationCount) {
        Q_EMIT infoMessage(i18np("One reservation imported.", "%1 reservations imported.", reservationCount));
    }
    if (passCount) {
        Q_EMIT infoMessage(i18np("One pass imported.", "%1 passes imported.", passCount));

    }
    if (healthCertCount) {
        Q_EMIT infoMessage(i18np("One health certificate imported.", "%1 health certificates imported.", healthCertCount));
    }
}

void ApplicationController::exportToFile(const QUrl &url)
{
    if (url.isEmpty()) {
        return;
    }

    qCDebug(Log) << url;
    File f(FileHelper::toLocalFile(url));
    if (!f.open(File::Write)) {
        qCWarning(Log) << f.errorString();
        Q_EMIT infoMessage(i18n("Export failed: %1", f.errorString()));
        return;
    }

    Exporter exporter(&f);
    exporter.exportReservations(m_resMgr);
    exporter.exportPasses(m_pkPassMgr);
    exporter.exportDocuments(m_docMgr);
    exporter.exportFavoriteLocations(m_favLocModel);
    exporter.exportTransfers(m_resMgr, m_transferMgr);
    exporter.exportPasses(m_passMgr);
    exporter.exportHealthCertificates(healthCertificateManager());
    exporter.exportLiveData();
    exporter.exportSettings();
    Q_EMIT infoMessage(i18n("Export completed."));
}

void ApplicationController::exportTripToFile(const QString &tripGroupId, const QUrl &url)
{
    if (url.isEmpty()) {
        return;
    }
    if (exportTripToFile(tripGroupId, FileHelper::toLocalFile(url))) {
        Q_EMIT infoMessage(i18n("Export completed."));
    }
}

void ApplicationController::exportTripToKDEConnect(const QString &tripGroupId, const QString &deviceId)
{
    if (tripGroupId.isEmpty() || deviceId.isEmpty()) {
        return;
    }

    if (!m_tempDir) {
        m_tempDir = std::make_unique<QTemporaryDir>();
    }

    QTemporaryFile f(m_tempDir->path() + QStringLiteral("/XXXXXX.itinerary"));
    if (!f.open()) {
        qCWarning(Log) << "Failed to open temporary file:" << f.errorString();
        Q_EMIT infoMessage(i18n("Export failed: %1", f.errorString()));
        return;
    }

    if (exportTripToFile(tripGroupId, f.fileName())) {
        // will be removed by the temporary dir it's in, as we don't know when the upload is done
        f.setAutoRemove(false);
        f.close();
        KDEConnect::sendToDevice(f.fileName(), deviceId);
    }
    Q_EMIT infoMessage(i18n("Trip sent."));
}

bool ApplicationController::exportTripToFile(const QString &tripGroupId, const QString &fileName)
{
    File f(fileName);
    if (!f.open(File::Write)) {
        qCWarning(Log) << f.errorString() << fileName;
        Q_EMIT infoMessage(i18n("Export failed: %1", f.errorString()));
        return false;
    }

    const auto tg = m_tripGroupMgr->tripGroup(tripGroupId);
    const auto batchIds = tg.elements();

    QSet<QString> docIdSet;

    Exporter exporter(&f);
    for (const auto &batchId : batchIds) {
        exporter.exportReservationBatch(m_resMgr, batchId);
        exporter.exportTransfersForBatch(batchId, m_transferMgr);
        exporter.exportLiveDataForBatch(batchId);

        const auto resIds = m_resMgr->reservationsForBatch(batchId);
        for (const auto &resId : resIds) {
            const auto res = m_resMgr->reservation(resId);

            const auto pkPassId = m_pkPassMgr->passId(res);
            exporter.exportPkPass(m_pkPassMgr, pkPassId);

            const auto docIds = DocumentUtil::documentIds(res);
            for (const auto &docId : docIds) {
                const auto id = docId.toString();
                if (!id.isEmpty() && m_docMgr->hasDocument(id) && !docIdSet.contains(id)) {
                    exporter.exportDocument(m_docMgr, id);
                    docIdSet.insert(id);
                }
            }
        }
    }
    return true;
}

void ApplicationController::exportTripToGpx(const QString &tripGroupId, const QUrl &url)
{
    if (url.isEmpty()) {
        return;
    }

    QFile f(FileHelper::toLocalFile(url));
    if (!f.open(QFile::WriteOnly)) {
        qCWarning(Log) << f.errorString() << f.fileName();
        Q_EMIT infoMessage(i18n("Export failed: %1", f.errorString()));
        return;
    }
    GpxExport exporter(&f);

    const auto tg = m_tripGroupMgr->tripGroup(tripGroupId);
    const auto batches = tg.elements();
    for (const auto &batchId : batches) {
        const auto res = m_resMgr->reservation(batchId);
        const auto transferBefore = m_transferMgr->transfer(batchId, Transfer::Before);
        const auto transferAfter = m_transferMgr->transfer(batchId, Transfer::After);
        exporter.writeReservation(res, m_liveDataMgr->journey(batchId), transferBefore, transferAfter);
    }
    Q_EMIT infoMessage(i18n("Export completed."));
}

void ApplicationController::exportBatchToFile(const QString &batchId, const QUrl &url)
{
    if (url.isEmpty()) {
        return;
    }
    if (exportBatchToFile(batchId, FileHelper::toLocalFile(url))) {
        Q_EMIT infoMessage(i18n("Export completed."));
    }
}

void ApplicationController::exportBatchToKDEConnect(const QString &batchId, const QString &deviceId)
{
    if (batchId.isEmpty() || deviceId.isEmpty()) {
        return;
    }

    if (!m_tempDir) {
        m_tempDir = std::make_unique<QTemporaryDir>();
    }

    QTemporaryFile f(m_tempDir->path() + QStringLiteral("/XXXXXX.itinerary"));
    if (!f.open()) {
        qCWarning(Log) << "Failed to open temporary file:" << f.errorString();
        Q_EMIT infoMessage(i18n("Export failed: %1", f.errorString()));
        return;
    }

    if (exportBatchToFile(batchId, f.fileName())) {
        // will be removed by the temporary dir it's in, as we don't know when the upload is done
        f.setAutoRemove(false);
        f.close();
        KDEConnect::sendToDevice(f.fileName(), deviceId);
    }
    Q_EMIT infoMessage(i18n("Reservation sent."));
}

bool ApplicationController::exportBatchToFile(const QString &batchId, const QString &fileName)
{
    File f(fileName);
    if (!f.open(File::Write)) {
        qCWarning(Log) << f.errorString() << fileName;
        Q_EMIT infoMessage(i18n("Export failed: %1", f.errorString()));
        return false;
    }

    QSet<QString> docIdSet;

    Exporter exporter(&f);
    exporter.exportReservationBatch(m_resMgr, batchId);
    exporter.exportTransfersForBatch(batchId, m_transferMgr);
    exporter.exportLiveDataForBatch(batchId);

    const auto resIds = m_resMgr->reservationsForBatch(batchId);
    for (const auto &resId : resIds) {
        const auto res = m_resMgr->reservation(resId);

        const auto pkPassId = PkPassManager::passId(res);
        exporter.exportPkPass(m_pkPassMgr, pkPassId);

        const auto docIds = DocumentUtil::documentIds(res);
        for (const auto &docId : docIds) {
            const auto id = docId.toString();
            if (!id.isEmpty() && m_docMgr->hasDocument(id) && !docIdSet.contains(id)) {
                exporter.exportDocument(m_docMgr, id);
                docIdSet.insert(id);
            }
        }
    }
    return true;
}

bool ApplicationController::importBundle(KItinerary::File *file)
{
    Importer importer(file);
    int count = 0;
    {
        QSignalBlocker blocker(this); // suppress infoMessage()
        count += importer.importReservations(m_resMgr);
        count += importer.importPasses(m_pkPassMgr);
        count += importer.importDocuments(m_docMgr);
        count += importer.importFavoriteLocations(m_favLocModel);
        count += importer.importTransfers(m_resMgr, m_transferMgr);
        count += importer.importPasses(m_passMgr);
        count += importer.importHealthCertificates(healthCertificateManager());
        count += importer.importLiveData(m_liveDataMgr);
        count += importer.importSettings();
    }

    if (count > 0) {
        Q_EMIT infoMessage(i18n("Import completed."));
    }
    return count > 0;
}

void ApplicationController::importPass(const QString &passId)
{
    if (m_importLock) {
        return;
    }

    const auto pass = m_pkPassMgr->pass(passId);
    KItinerary::ExtractorEngine engine;
    engine.setContent(QVariant::fromValue<KPkPass::Pass*>(pass), u"application/vnd.apple.pkpass");
    const auto resIds = m_resMgr->addReservationsWithPostProcessing(JsonLdDocument::fromJson(engine.extract()));
    if (!resIds.isEmpty()) {
        Q_EMIT infoMessage(i18np("One reservation imported.", "%1 reservations imported.", resIds.size()));
    }
}

QString ApplicationController::addDocumentFromFile(const QUrl &url)
{
    if (!url.isValid()) {
        return {};
    }

    const auto docId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    DigitalDocument docInfo;
#ifdef Q_OS_ANDROID
    docInfo.setEncodingFormat(KAndroidExtras::ContentResolver::mimeType(url));
    docInfo.setName(KAndroidExtras::ContentResolver::fileName(url));
#else
    QMimeDatabase db;
    docInfo.setEncodingFormat(db.mimeTypeForFile(FileHelper::toLocalFile(url)).name());
    docInfo.setName(url.fileName());
#endif

    m_docMgr->addDocument(docId, docInfo, FileHelper::toLocalFile(url));
    return docId;
}

void ApplicationController::addDocumentToReservation(const QString &batchId, const QUrl &url)
{
    const auto docId = addDocumentFromFile(url);
    if (docId.isEmpty()) {
        return;
    }
    const auto resIds = m_resMgr->reservationsForBatch(batchId);
    for (const auto &resId : resIds) {
        auto res = m_resMgr->reservation(resId);
        DocumentUtil::addDocumentId(res, docId);
        m_resMgr->updateReservation(resId, res);
    }
}

void ApplicationController::addDocumentToPass(const QString &passId, const QUrl &url)
{
    const auto docId = addDocumentFromFile(url);
    if (docId.isEmpty()) {
        return;
    }
    auto pass = m_passMgr->pass(passId);
    if (DocumentUtil::addDocumentId(pass, docId)) {
        m_passMgr->update(passId, pass);
    }
}

void ApplicationController::removeDocumentFromReservation(const QString &batchId, const QString &docId)
{
    const auto resIds = m_resMgr->reservationsForBatch(batchId);
    for (const auto &resId : resIds) {
        auto res = m_resMgr->reservation(batchId);
        if (DocumentUtil::removeDocumentId(res, docId)) {
            m_resMgr->updateReservation(resId, res);
        }
    }
    m_docMgr->removeDocument(docId);
}

void ApplicationController::removeDocumentFromPass(const QString &passId, const QString &docId)
{
    auto pass = m_passMgr->pass(passId);
    if (DocumentUtil::removeDocumentId(pass, docId)) {
        m_passMgr->update(passId, pass);
    }
    m_docMgr->removeDocument(docId);
}

void ApplicationController::openDocument(const QUrl &url)
{
#ifdef Q_OS_ANDROID
    using namespace KAndroidExtras;
    auto uri = ItineraryActivity().openDocument(url.toLocalFile());

    Intent intent;
    intent.setData(uri);
    intent.setAction(Intent::ACTION_VIEW);
    intent.addFlags(Intent::FLAG_GRANT_READ_URI_PERMISSION);
    Activity::startActivity(intent, 0);
#else
    QDesktopServices::openUrl(url);
#endif
}

QString ApplicationController::applicationVersion() const
{
    return QString::fromUtf8(ITINERARY_DETAILED_VERSION_STRING);
}

QString ApplicationController::extractorCapabilities() const
{
    return ExtractorCapabilities::capabilitiesString();
}

QVariant ApplicationController::aboutData() const
{
    return QVariant::fromValue(KAboutData::applicationData());
}

QString ApplicationController::userAgent()
{
    return QLatin1StringView("org.kde.itinerary/") + QCoreApplication::applicationVersion() + QLatin1StringView(" (kde-pim@kde.org)");
}

bool ApplicationController::hasHealthCertificateSupport() const
{
    return HealthCertificateManager::isAvailable();
}

HealthCertificateManager* ApplicationController::healthCertificateManager() const
{
    if (!m_healthCertMgr) {
        m_healthCertMgr = new HealthCertificateManager(const_cast<ApplicationController*>(this));
    }
    return m_healthCertMgr;
}

#include "moc_applicationcontroller.cpp"
