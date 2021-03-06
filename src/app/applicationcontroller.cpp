/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "applicationcontroller.h"
#include "documentmanager.h"
#include "favoritelocationmodel.h"
#include "gpxexport.h"
#include "healthcertificatemanager.h"
#include "importexport.h"
#include "livedatamanager.h"
#include "logging.h"
#include "pkpassmanager.h"
#include "reservationmanager.h"
#include "transfermanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"
#include <itinerary_version_detailed.h>

#include <KItinerary/CreativeWork>
#include <KItinerary/DocumentUtil>
#include <KItinerary/ExtractorCapabilities>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/File>
#include <KItinerary/JsonLdDocument>

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
#include <QStandardPaths>
#include <QUuid>
#include <QUrl>
#include <QUrlQuery>

#ifdef Q_OS_ANDROID
#include <KMime/Message>
#include <KMime/Types>

#include <kandroidextras/activity.h>
#include <kandroidextras/contentresolver.h>
#include <kandroidextras/intent.h>
#include <kandroidextras/jniarray.h>
#include <kandroidextras/jnisignature.h>
#include <kandroidextras/jnitypes.h>
#include <kandroidextras/manifestpermission.h>
#include <kandroidextras/uri.h>

#include <QtAndroid>
#include <QAndroidJniObject>
#endif

#include <memory>

using namespace KItinerary;

#ifdef Q_OS_ANDROID

static void importDavDroidJson(JNIEnv *env, jobject that, jstring data)
{
    Q_UNUSED(that)
    const char *str = env->GetStringUTFChars(data, nullptr);
    const auto doc = QJsonDocument::fromJson(str);
    env->ReleaseStringUTFChars(data, str);

    const auto array = doc.array();
    if (array.size() < 2 || array.at(0).toString() != QLatin1String("X-KDE-KITINERARY-RESERVATION")) {
        return;
    }

    auto propValue = array.at(1).toString().toUtf8();
    // work around ical/JSON mis-encoding with newer DAVx⁵ versions
    propValue.replace("\\,", ",");

    ApplicationController::instance()->importData(propValue);
}

static void importFromIntent(JNIEnv *env, jobject that, jobject data)
{
    Q_UNUSED(that)
    Q_UNUSED(env)
    ApplicationController::instance()->importFromIntent(KAndroidExtras::Intent(data));
}

static const JNINativeMethod methods[] = {
    {"importFromIntent", "(Landroid/content/Intent;)V", (void*)importFromIntent},
    {"importDavDroidJson", "(Ljava/lang/String;)V", (void*)importDavDroidJson}
};

Q_DECL_EXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void*)
{
    static bool initialized = false;
    if (initialized)
        return JNI_VERSION_1_6;
    initialized = true;

    JNIEnv *env = nullptr;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
        qCWarning(Log) << "Failed to get JNI environment.";
        return -1;
    }
    jclass cls = env->FindClass("org/kde/itinerary/Activity");
    if (env->RegisterNatives(cls, methods, sizeof(methods) / sizeof(JNINativeMethod)) < 0) {
        qCWarning(Log) << "Failed to register native functions.";
        return -1;
    }

    return JNI_VERSION_1_4;
}
#endif

ApplicationController* ApplicationController::s_instance = nullptr;

ApplicationController::ApplicationController(QObject* parent)
    : QObject(parent)
{
    s_instance = this;

    connect(QGuiApplication::clipboard(), &QClipboard::dataChanged, this, &ApplicationController::clipboardContentChanged);
}

ApplicationController::~ApplicationController()
{
    s_instance = nullptr;
}

ApplicationController* ApplicationController::instance()
{
    return s_instance;
}

void ApplicationController::setReservationManager(ReservationManager* resMgr)
{
    m_resMgr = resMgr;
}

void ApplicationController::setPkPassManager(PkPassManager* pkPassMgr)
{
    m_pkPassMgr = pkPassMgr;
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

void ApplicationController::setHealthCertificateManager(HealthCertificateManager *healthCertMgr)
{
    m_healthCertMgr = healthCertMgr;
}

#ifdef Q_OS_ANDROID
void ApplicationController::importFromIntent(const KAndroidExtras::Intent &intent)
{
    using namespace KAndroidExtras;
    const auto action = intent.getAction();

    // main entry point, nothing to do
    if (action == Intent::ACTION_MAIN) {
        return;
    }

    // opening a file
    if (action == Intent::ACTION_VIEW) {
        importFromUrl(intent.getData());
        return;
    }

    // shared data, e.g. from email applications like FairMail
    if (action == Intent::ACTION_SEND || action == Intent::ACTION_SEND_MULTIPLE) {
        const auto type = intent.getType();
        const auto subject = intent.getStringExtra(Intent::EXTRA_SUBJECT);
        const auto from = intent.getStringArrayExtra(Intent::EXTRA_EMAIL);
        const auto text = intent.getStringExtra(Intent::EXTRA_TEXT);
        qCInfo(Log) << action << type << subject << from << text;
        const auto attachments = Jni::fromArray<QStringList>(QtAndroid::androidActivity().callObjectMethod("attachmentsForIntent", Jni::signature<Jni::Array<java::lang::String>(android::content::Intent)>(), static_cast<QAndroidJniObject>(intent).object()));
        qCInfo(Log) << attachments;

        KMime::Message msg;
        msg.subject()->fromUnicodeString(subject, "utf-8");
        for (const auto &f : from) {
            KMime::Types::Mailbox mb;
            mb.fromUnicodeString(f);
            msg.from()->addAddress(mb);
        }

        if (attachments.empty()) {
            msg.contentType()->setMimeType(type.toUtf8());
            msg.setBody(text.toUtf8());
        } else {
            auto body = new KMime::Content;
            body->contentType()->setMimeType(type.toUtf8());
            body->setBody(text.toUtf8());
            msg.addContent(body);
            for (const auto &a : attachments) {
                auto att = new KMime::Content;
                att->contentType()->setMimeType(ContentResolver::mimeType(QUrl(a)).toUtf8());
                QFile f(a);
                if (!f.open(QFile::ReadOnly)) {
                    qCWarning(Log) << "Failed to open attachement:" << a << f.errorString();
                    continue;
                }
                att->setBody(f.readAll());
                msg.addContent(att);
            }
        }

        msg.assemble();
        qDebug().noquote() << msg.encodedContent();
        m_resMgr->importReservation(&msg);
        return;
    }

    qCInfo(Log) << "Unhandled intent action:" << action;
}
#endif

void ApplicationController::importFromClipboard()
{
    const auto md = QGuiApplication::clipboard()->mimeData();
    if (md->hasUrls()) {
        const auto urls = md->urls();
        for (const auto &url : urls) {
            importFromUrl(url);
        }
    }

    else if (md->hasText()) {
        const auto content = md->data(QLatin1String("text/plain"));
        importData(content);
    }

    else if (md->hasFormat(QLatin1String("application/octet-stream"))) {
        importData(md->data(QLatin1String("application/octet-stream")));
    }
}

void ApplicationController::importFromUrl(const QUrl &url)
{
    if (!url.isValid()) {
        return;
    }

    qCDebug(Log) << url;
    if (url.isLocalFile() || url.scheme() == QLatin1String("content")) {
        importLocalFile(url);
        return;
    }

    if (url.scheme().startsWith(QLatin1String("http"))) {
        if (!m_nam ) {
            m_nam = new QNetworkAccessManager(this);
        }
        auto reqUrl(url);
        reqUrl.setScheme(QLatin1String("https"));
        QNetworkRequest req(reqUrl);
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        auto reply = m_nam->get(req);
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() != QNetworkReply::NoError) {
                qCDebug(Log) << reply->url() << reply->errorString();
                return;
            }
            importData(reply->readAll());
        });
        return;
    }

    qCDebug(Log) << "Unhandled URL type:" << url;
}

void ApplicationController::importLocalFile(const QUrl &url)
{
    qCDebug(Log) << url;
    if (url.isEmpty()) {
        return;
    }

    QFile f(url.isLocalFile() ? url.toLocalFile() : url.toString());
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to open" << f.fileName() << f.errorString();
        return;
    }
    if (f.size() > 4000000 && !f.fileName().endsWith(QLatin1String(".itinerary"))) {
        qCWarning(Log) << "File too large, ignoring" << f.fileName() << f.size();
        return;
    }

    const auto head = f.peek(4);
    if (url.fileName().endsWith(QLatin1String(".pkpass"), Qt::CaseInsensitive)) {
        m_pkPassMgr->importPass(url);
    } else if (url.fileName().endsWith(QLatin1String(".itinerary"), Qt::CaseInsensitive)) {
        importBundle(url);
    } else if (strncmp(head.constData(), "PK\x03\x04", 4) == 0) {
        if (m_pkPassMgr->importPass(url).isEmpty()) {
            importBundle(url);
        }
    } else {
        const auto data = f.readAll();
        const auto resIds = importReservationOrHealthCertificate(data, f.fileName());
        if (resIds.empty()) {
            return;
        }

        // check if there is a document we want to attach here
        QMimeDatabase db;
        const auto mt = db.mimeTypeForFileNameAndData(f.fileName(), data);
        if (mt.name() != QLatin1String("application/pdf")) { // TODO support more file types (however we certainly want to exclude pkpass and json here)
            return;
        }

        DigitalDocument docInfo;
        docInfo.setName(f.fileName());
        docInfo.setEncodingFormat(mt.name());
        const auto docId = DocumentUtil::idForContent(data);
        m_docMgr->addDocument(docId, docInfo, data);

        for (const auto &resId : resIds) {
            auto res = m_resMgr->reservation(resId);
            if (DocumentUtil::addDocumentId(res, docId)) {
                m_resMgr->updateReservation(resId, res);
            }
        }
    }
}

void ApplicationController::importData(const QByteArray &data)
{
    qCDebug(Log);
    if (data.size() < 4) {
        return;
    }
    if (strncmp(data.constData(), "PK\x03\x04", 4) == 0) {
        if (m_pkPassMgr->importPassFromData(data).isEmpty()) {
            importBundle(data);
        }
    } else {
        importReservationOrHealthCertificate(data);
    }
}

void ApplicationController::checkCalendar()
{
#ifdef Q_OS_ANDROID
    using namespace KAndroidExtras;
    if (QtAndroid::checkPermission(ManifestPermission::READ_CALENDAR) == QtAndroid::PermissionResult::Granted) {
        const auto activity = QtAndroid::androidActivity();
        if (activity.isValid()) {
            activity.callMethod<void>("checkCalendar");
        }
    } else {
        QtAndroid::requestPermissions({ManifestPermission::READ_CALENDAR}, [this] (const QtAndroid::PermissionResultMap &result){
            if (result[ManifestPermission::READ_CALENDAR] == QtAndroid::PermissionResult::Granted) {
                checkCalendar();
            }
        });
    }
#endif
}

bool ApplicationController::hasClipboardContent() const
{
    const auto md = QGuiApplication::clipboard()->mimeData();
    return md->hasText() || md->hasUrls() || md->hasFormat(QLatin1String("application/octet-stream"));
}


void ApplicationController::exportToFile(const QString &filePath)
{
    qCDebug(Log) << filePath;
    if (filePath.isEmpty()) {
        return;
    }

    File f(QUrl(filePath).isLocalFile() ? QUrl(filePath).toLocalFile() : filePath);
    if (!f.open(File::Write)) {
        qCWarning(Log) << f.errorString();
        // TODO show error in UI
        return;
    }

    Exporter exporter(&f);
    exporter.exportReservations(m_resMgr);
    exporter.exportPasses(m_pkPassMgr);
    exporter.exportDocuments(m_docMgr);
    exporter.exportFavoriteLocations(m_favLocModel);
    exporter.exportTransfers(m_resMgr, m_transferMgr);
    exporter.exportHealthCertificates(m_healthCertMgr);
    exporter.exportLiveData();
    exporter.exportSettings();
}

void ApplicationController::exportTripToGpx(const QString &tripGroupId, const QString &filePath)
{
    if (filePath.isEmpty()) {
        return;
    }

    QFile f(QUrl(filePath).isLocalFile() ? QUrl(filePath).toLocalFile() : filePath);
    if (!f.open(QFile::WriteOnly)) {
        qCWarning(Log) << f.errorString() << f.fileName();
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
}

void ApplicationController::importBundle(const QUrl &url)
{
    KItinerary::File f(url.isLocalFile() ? url.toLocalFile() : url.toString());
    if (!f.open(File::Read)) {
        // TODO show error in the ui
        qCWarning(Log) << "Failed to open bundle file:" << url << f.errorString();
        return;
    }

    importBundle(&f);
}

void ApplicationController::importBundle(const QByteArray &data)
{
    QBuffer buffer;
    buffer.setData(data);
    buffer.open(QBuffer::ReadOnly);
    KItinerary::File f(&buffer);
    if (!f.open(File::Read)) {
        // TODO show error in the ui
        qCWarning(Log) << "Failed to open bundle data:" << f.errorString();
        return;
    }

    importBundle(&f);
}

void ApplicationController::importBundle(KItinerary::File *file)
{
    Importer importer(file);
    importer.importReservations(m_resMgr);
    importer.importPasses(m_pkPassMgr);
    importer.importDocuments(m_docMgr);
    importer.importFavoriteLocations(m_favLocModel);
    importer.importTransfers(m_resMgr, m_transferMgr);
    importer.importHealthCertificates(m_healthCertMgr);
    importer.importLiveData(m_liveDataMgr);
    importer.importSettings();

    // favorite locations
    auto favLocs = FavoriteLocation::fromJson(QJsonDocument::fromJson(file->customData(QStringLiteral("org.kde.itinerary/favorite-locations"), QStringLiteral("locations"))).array());
    if (!favLocs.empty()) {
        m_favLocModel->setFavoriteLocations(std::move(favLocs));
    }
}

QVector<QString> ApplicationController::importReservationOrHealthCertificate(const QByteArray &data, const QString &fileName)
{
    using namespace KItinerary;
    ExtractorEngine engine;
    engine.setContextDate(QDateTime(QDate::currentDate(), QTime(0, 0)));
    engine.setData(data, fileName);
    const auto resIds = m_resMgr->importReservations(JsonLdDocument::fromJson(engine.extract()));
    if (!resIds.isEmpty()) {
        return resIds;
    }

    // look for health certificate barcodes instead
    importHealthCertificateRecursive(engine.rootDocumentNode());

    return {};
}

void ApplicationController::importHealthCertificateRecursive(const ExtractorDocumentNode &node)
{
    if (node.childNodes().size() == 1 && node.mimeType() == QLatin1String("internal/qimage")) {
        const auto &child = node.childNodes()[0];
        if (child.isA<QString>()) {
            return m_healthCertMgr->importCertificate(child.content<QString>().toUtf8());
        }
        if (child.isA<QByteArray>()) {
            return m_healthCertMgr->importCertificate(child.content<QByteArray>());
        }
    }

    for (const auto &child : node.childNodes()) {
        importHealthCertificateRecursive(child);
    }
}

void ApplicationController::addDocument(const QString &batchId, const QUrl &url)
{
    if (!url.isValid()) {
        return;
    }

    const auto docId = QUuid::createUuid().toString();
    auto res = m_resMgr->reservation(batchId);
    DocumentUtil::addDocumentId(res, docId);

    DigitalDocument docInfo;
#ifdef Q_OS_ANDROID
    docInfo.setEncodingFormat(KAndroidExtras::ContentResolver::mimeType(url));
    docInfo.setName(KAndroidExtras::ContentResolver::fileName(url));
#else
    QMimeDatabase db;
    docInfo.setEncodingFormat(db.mimeTypeForFile(url.isLocalFile() ? url.toLocalFile() : url.toString()).name());
    docInfo.setName(url.fileName());
#endif

    m_docMgr->addDocument(docId, docInfo, url.isLocalFile() ? url.toLocalFile() : url.toString());

    const auto resIds = m_resMgr->reservationsForBatch(batchId);
    for (const auto &resId : resIds) {
        m_resMgr->updateReservation(resId, res);
    }
}

void ApplicationController::removeDocument(const QString &batchId, const QString &docId)
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

void ApplicationController::openDocument(const QUrl &url)
{
#ifdef Q_OS_ANDROID
    using namespace KAndroidExtras;
    auto activity = QtAndroid::androidActivity();
    auto uri = activity.callObjectMethod("openDocument", Jni::signature<android::net::Uri(java::lang::String)>(), QAndroidJniObject::fromString(url.toLocalFile()).object());

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
