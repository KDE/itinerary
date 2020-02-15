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

#include "applicationcontroller.h"
#include "documentmanager.h"
#include "favoritelocationmodel.h"
#include "logging.h"
#include "pkpassmanager.h"
#include "reservationmanager.h"
#include "transfermanager.h"
#include <itinerary_version_detailed.h>

#include <KItinerary/CreativeWork>
#include <KItinerary/DocumentUtil>
#include <KItinerary/ExtractorCapabilities>
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
#include <kandroidextras/activity.h>
#include <kandroidextras/contentresolver.h>
#include <kandroidextras/intent.h>
#include <kandroidextras/uri.h>
#include <kandroidextras/jnisignature.h>
#include <kandroidextras/jnitypes.h>

#include <QtAndroid>
#include <QAndroidJniObject>
#else
#include <QFileDialog>
#endif

#include <memory>

using namespace KItinerary;

#ifdef Q_OS_ANDROID

#define PERMISSION_CALENDAR QStringLiteral("android.permission.READ_CALENDAR")

static void importReservation(JNIEnv *env, jobject that, jstring data)
{
    Q_UNUSED(that);
    const char *str = env->GetStringUTFChars(data, nullptr);
    ApplicationController::instance()->importData(str);
    env->ReleaseStringUTFChars(data, str);
}

static void importDavDroidJson(JNIEnv *env, jobject that, jstring data)
{
    Q_UNUSED(that);
    const char *str = env->GetStringUTFChars(data, nullptr);
    const auto doc = QJsonDocument::fromJson(str);
    env->ReleaseStringUTFChars(data, str);

    const auto array = doc.array();
    if (array.size() < 2 || array.at(0).toString() != QLatin1String("X-KDE-KITINERARY-RESERVATION")) {
        return;
    }

    ApplicationController::instance()->importData(array.at(1).toString().toUtf8());
}

static void importFromIntent(JNIEnv *env, jobject that, jobject data)
{
    Q_UNUSED(that)
    Q_UNUSED(env)

    KAndroidExtras::Intent intent(data);
    ApplicationController::instance()->importFromUrl(intent.getData());
}

static const JNINativeMethod methods[] = {
    {"importReservation", "(Ljava/lang/String;)V", (void*)importReservation},
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

void ApplicationController::importFromClipboard()
{
    if (QGuiApplication::clipboard()->mimeData()->hasUrls()) {
        const auto urls = QGuiApplication::clipboard()->mimeData()->urls();
        for (const auto &url : urls)
            importFromUrl(url);
        return;
    }

    if (QGuiApplication::clipboard()->mimeData()->hasText()) {
        const auto content = QGuiApplication::clipboard()->mimeData()->data(QLatin1String("text/plain"));
        importData(content);
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
    if (f.size() > 4000000) {
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
        const auto resIds = m_resMgr->importReservation(data, f.fileName());
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
        m_resMgr->importReservation(data);
    }
}

void ApplicationController::checkCalendar()
{
#ifdef Q_OS_ANDROID

    if (QtAndroid::checkPermission(PERMISSION_CALENDAR) == QtAndroid::PermissionResult::Granted) {
        const auto activity = QtAndroid::androidActivity();
        if (activity.isValid()) {
            activity.callMethod<void>("checkCalendar");
        }
    } else {
        QtAndroid::requestPermissions({PERMISSION_CALENDAR}, [this] (const QtAndroid::PermissionResultMap &result){
            if (result[PERMISSION_CALENDAR] == QtAndroid::PermissionResult::Granted) {
                checkCalendar();
            }
        });
    }
#endif
}

bool ApplicationController::hasClipboardContent() const
{
    return QGuiApplication::clipboard()->mimeData()->hasText() || QGuiApplication::clipboard()->mimeData()->hasUrls();
}

void ApplicationController::exportData()
{
    qCDebug(Log);
#ifdef Q_OS_ANDROID
    using namespace KAndroidExtras;
    Intent intent;
    intent.setAction(Intent::ACTION_CREATE_DOCUMENT());
    intent.addCategory(Intent::CATEGORY_OPENABLE());
    intent.setType(QStringLiteral("*/*"));
    QtAndroid::startActivity(intent, 0, [this](int, int, const QAndroidJniObject &jniIntent) {
        Intent intent(jniIntent);
        exportToFile(intent.getData().toString());
    });
#else
    const auto filePath = QFileDialog::getSaveFileName(nullptr, i18n("Export Itinerary Data"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        i18n("KDE Itinerary files (*.itinerary)"));
    exportToFile(filePath);
#endif
}

void ApplicationController::exportToFile(const QString &filePath)
{
    qCDebug(Log) << filePath;
    if (filePath.isEmpty()) {
        return;
    }

    File f(filePath);
    if (!f.open(File::Write)) {
        qCWarning(Log) << f.errorString();
        // TODO show error in UI
        return;
    }

    // export reservation data
    for (const auto &batchId : m_resMgr->batches()) {
        const auto resIds = m_resMgr->reservationsForBatch(batchId);
        for (const auto &resId : resIds) {
            f.addReservation(resId, m_resMgr->reservation(resId));
        }
    }

    // export passes
    const auto passIds = m_pkPassMgr->passes();
    for (const auto &passId : passIds) {
        f.addPass(passId, m_pkPassMgr->rawData(passId));
    }

    // export documents
    const auto docIds = m_docMgr->documents();
    for (const auto &docId : docIds) {
        const auto fileName = m_docMgr->documentFilePath(docId);
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly)) {
            qCWarning(Log) << "failed to open" << fileName << "for exporting" << file.errorString();
            continue;
        }
        f.addDocument(docId, m_docMgr->documentInfo(docId), file.readAll());
    }

    // export transfer elements
    const auto transferDomain = QStringLiteral("org.kde.itinerary/transfers");
    for (const auto &batchId : m_resMgr->batches()) {
        auto t = m_transferMgr->transfer(batchId, Transfer::Before);
        if (t.state() != Transfer::UndefinedState) {
            f.addCustomData(transferDomain, t.identifier(), QJsonDocument(Transfer::toJson(t)).toJson());
        }
        t = m_transferMgr->transfer(batchId, Transfer::After);
        if (t.state() != Transfer::UndefinedState) {
            f.addCustomData(transferDomain, t.identifier(), QJsonDocument(Transfer::toJson(t)).toJson());
        }
    }

    // export favorite locations
    if (m_favLocModel->rowCount() > 0) {
        f.addCustomData(QStringLiteral("org.kde.itinerary/favorite-locations"), QStringLiteral("locations"),
                        QJsonDocument(FavoriteLocation::toJson(m_favLocModel->favoriteLocations())).toJson());
    }

    // TODO export settings
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
    const auto resIds = file->reservations();
    for (const auto &resId : resIds) {
        m_resMgr->addReservation(file->reservation(resId));
    }

    const auto passIds = file->passes();
    for (const auto &passId : passIds) {
        m_pkPassMgr->importPassFromData(file->passData(passId));
    }

    const auto docIds = file->documents();
    for (const auto &docId : docIds) {
        m_docMgr->addDocument(docId, file->documentInfo(docId), file->documentData(docId));
    }

    // import transfers
    const auto transferDomain = QStringLiteral("org.kde.itinerary/transfers");
    for (const auto &batchId : m_resMgr->batches()) {
        auto t = Transfer::fromJson(QJsonDocument::fromJson(file->customData(transferDomain, Transfer::identifier(batchId, Transfer::Before))).object());
        m_transferMgr->importTransfer(t);
        t = Transfer::fromJson(QJsonDocument::fromJson(file->customData(transferDomain, Transfer::identifier(batchId, Transfer::After))).object());
        m_transferMgr->importTransfer(t);
    }

    // favorite locations
    auto favLocs = FavoriteLocation::fromJson(QJsonDocument::fromJson(file->customData(QStringLiteral("org.kde.itinerary/favorite-locations"), QStringLiteral("locations"))).array());
    if (!favLocs.empty()) {
        m_favLocModel->setFavoriteLocations(std::move(favLocs));
    }
}

void ApplicationController::addDocument(const QString &batchId)
{
#ifndef Q_OS_ANDROID
    const auto url = QFileDialog::getOpenFileUrl(nullptr, i18n("Add Document"),
        QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
        i18n("All Files (*.*)"));
    addDocument(batchId, url);
#else
    using namespace KAndroidExtras;
    Intent intent;
    intent.setAction(Intent::ACTION_OPEN_DOCUMENT());
    intent.addCategory(Intent::CATEGORY_OPENABLE());
    intent.setType(QStringLiteral("*/*"));
    QtAndroid::startActivity(intent, 0, [this, batchId](int, int, const QAndroidJniObject &jniIntent) {
        Intent intent(jniIntent);
        addDocument(batchId, intent.getData());
    });
#endif
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
    intent.setAction(Intent::ACTION_VIEW());
    intent.addFlags(Intent::FLAG_GRANT_READ_URI_PERMISSION());
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
