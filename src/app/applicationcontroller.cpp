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
#include "logging.h"
#include "pkpassmanager.h"
#include "reservationmanager.h"

#include <KItinerary/JsonLdDocument>
#include <KItinerary/Place>

#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QMimeData>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniObject>
#else
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#endif

#include <memory>

using namespace KItinerary;

#ifdef Q_OS_ANDROID

#define PERMISSION_CALENDAR QStringLiteral("android.permission.READ_CALENDAR")

static void importReservation(JNIEnv *env, jobject that, jstring data)
{
    Q_UNUSED(that);
    ApplicationController::instance()->importData(env->GetStringUTFChars(data, 0));
}

static void importFromIntent(JNIEnv *env, jobject that, jobject data)
{
    Q_UNUSED(that);
    Q_UNUSED(env)
    ApplicationController::instance()->importFromIntent(data);
}

static const JNINativeMethod methods[] = {
    {"importReservation", "(Ljava/lang/String;)V", (void*)importReservation},
    {"importFromIntent", "(Landroid/content/Intent;)V", (void*)importFromIntent}
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
#ifdef Q_OS_ANDROID
    , m_activityResultReceiver(this)
#endif
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

void ApplicationController::showOnMap(const QVariant &place)
{
    if (place.isNull()) {
        return;
    }

    const auto geo = JsonLdDocument::readProperty(place, "geo").value<GeoCoordinates>();
    const auto addr = JsonLdDocument::readProperty(place, "address").value<PostalAddress>();

#ifdef Q_OS_ANDROID
    QString intentUri;
    if (geo.isValid()) {
        intentUri = QLatin1String("geo:") + QString::number(geo.latitude())
            + QLatin1Char(',') + QString::number(geo.longitude());
    } else if (!addr.isEmpty()) {
        intentUri = QLatin1String("geo:0,0?q=") + addr.streetAddress() + QLatin1String(", ")
            + addr.postalCode() + QLatin1Char(' ')
            + addr.addressLocality() + QLatin1String(", ")
            + addr.addressCountry();
    } else {
        return;
    }

    const auto activity = QtAndroid::androidActivity();
    if (activity.isValid()) {
        activity.callMethod<void>("launchViewIntentFromUri", "(Ljava/lang/String;)V", QAndroidJniObject::fromString(intentUri).object());
    }

#else
    if (geo.isValid()) {
        // zoom out further from airports, they are larger and you usually want to go further away from them
        const auto zoom = place.userType() == qMetaTypeId<Airport>() ? 12 : 17;
        QUrl url;
        url.setScheme(QStringLiteral("https"));
        url.setHost(QStringLiteral("www.openstreetmap.org"));
        url.setPath(QStringLiteral("/"));
        const QString fragment = QLatin1String("map=") + QString::number(zoom)
                                    + QLatin1Char('/') + QString::number(geo.latitude())
                                    + QLatin1Char('/') + QString::number(geo.longitude());
        url.setFragment(fragment);
        QDesktopServices::openUrl(url);
        return;
    }

    if (!addr.isEmpty()) {
        QUrl url;
        url.setScheme(QStringLiteral("https"));
        url.setHost(QStringLiteral("www.openstreetmap.org"));
        url.setPath(QStringLiteral("/search"));
        const QString queryString = addr.streetAddress() + QLatin1String(", ")
                                    + addr.postalCode() + QLatin1Char(' ')
                                    + addr.addressLocality() + QLatin1String(", ")
                                    + addr.addressCountry();
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("query"), queryString);
        url.setQuery(query);
        QDesktopServices::openUrl(url);
    }
#endif
}

bool ApplicationController::canNavigateTo(const QVariant& place)
{
    if (place.isNull()) {
        return false;
    }

    if (JsonLdDocument::readProperty(place, "geo").value<GeoCoordinates>().isValid()) {
        return true;
    }

#ifdef Q_OS_ANDROID
    return !JsonLdDocument::readProperty(place, "address").value<PostalAddress>().isEmpty();
#else
    return false;
#endif
}

void ApplicationController::navigateTo(const QVariant& place)
{
    if (place.isNull()) {
        return;
    }

#ifdef Q_OS_ANDROID
    const auto geo = JsonLdDocument::readProperty(place, "geo").value<GeoCoordinates>();
    const auto addr = JsonLdDocument::readProperty(place, "address").value<PostalAddress>();

    QString intentUri;
    if (geo.isValid()) {
        intentUri = QLatin1String("google.navigation:q=") + QString::number(geo.latitude())
            + QLatin1Char(',') + QString::number(geo.longitude());
    } else if (!addr.isEmpty()) {
        intentUri = QLatin1String("google.navigation:q=") + addr.streetAddress() + QLatin1String(", ")
            + addr.postalCode() + QLatin1Char(' ')
            + addr.addressLocality() + QLatin1String(", ")
            + addr.addressCountry();
    } else {
        return;
    }

    const auto activity = QtAndroid::androidActivity();
    if (activity.isValid()) {
        activity.callMethod<void>("launchViewIntentFromUri", "(Ljava/lang/String;)V", QAndroidJniObject::fromString(intentUri).object());
    }

#else
    if (m_pendingNavigation) {
        return;
    }

    if (!m_positionSource) {
        m_positionSource = QGeoPositionInfoSource::createDefaultSource(this);
        if (!m_positionSource) {
            qWarning() << "no geo position info source available";
            return;
        }
    }

    if (m_positionSource->lastKnownPosition().isValid()) {
        navigateTo(m_positionSource->lastKnownPosition(), place);
    } else {
        m_pendingNavigation = connect(m_positionSource, &QGeoPositionInfoSource::positionUpdated,
                                      this, [this, place](const QGeoPositionInfo &pos) {
            navigateTo(pos, place);
        });
        m_positionSource->requestUpdate();
    }
#endif
}

#ifndef Q_OS_ANDROID
void ApplicationController::navigateTo(const QGeoPositionInfo &from, const QVariant &to)
{
    qDebug() << from.coordinate() << from.isValid();
    disconnect(m_pendingNavigation);
    if (!from.isValid()) {
        return;
    }

    const auto geo = JsonLdDocument::readProperty(to, "geo").value<GeoCoordinates>();
    if (geo.isValid()) {
        QUrl url;
        url.setScheme(QStringLiteral("https"));
        url.setHost(QStringLiteral("www.openstreetmap.org"));
        url.setPath(QStringLiteral("/directions"));
        QUrlQuery query;
        query.addQueryItem(QLatin1String("route"),
            QString::number(from.coordinate().latitude()) + QLatin1Char(',') + QString::number(from.coordinate().longitude())
            + QLatin1Char(';') + QString::number(geo.latitude()) + QLatin1Char(',') + QString::number(geo.longitude()));
        url.setQuery(query);
        QDesktopServices::openUrl(url);
        return;
    }
}
#endif

#ifdef Q_OS_ANDROID
void ApplicationController::importFromIntent(const QAndroidJniObject &intent)
{
    if (!intent.isValid()) {
        return;
    }

    const auto uri = intent.callObjectMethod("getData", "()Landroid/net/Uri;");
    if (!uri.isValid()) {
        return;
    }

    const auto uriStr = uri.callObjectMethod("toString", "()Ljava/lang/String;");
    importFromUrl(QUrl(uriStr.toString()));
}

void ApplicationController::ActivityResultReceiver::handleActivityResult(int receiverRequestCode, int resultCode, const QAndroidJniObject &intent)
{
    qCDebug(Log) << receiverRequestCode << resultCode;
    m_controller->importFromIntent(intent);
}
#endif

void ApplicationController::showImportFileDialog()
{
#ifdef Q_OS_ANDROID
    const auto ACTION_OPEN_DOCUMENT = QAndroidJniObject::getStaticObjectField<jstring>("android/content/Intent", "ACTION_OPEN_DOCUMENT");
    QAndroidJniObject intent("android/content/Intent", "(Ljava/lang/String;)V", ACTION_OPEN_DOCUMENT.object());
    const auto CATEGORY_OPENABLE = QAndroidJniObject::getStaticObjectField<jstring>("android/content/Intent", "CATEGORY_OPENABLE");
    intent.callObjectMethod("addCategory", "(Ljava/lang/String;)Landroid/content/Intent;", CATEGORY_OPENABLE.object());
    intent.callObjectMethod("setType", "(Ljava/lang/String;)Landroid/content/Intent;", QAndroidJniObject::fromString(QStringLiteral("*/*")).object());
    QtAndroid::startActivity(intent, 0, &m_activityResultReceiver);
#endif
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
    if (url.fileName().endsWith(QLatin1String(".pkpass"), Qt::CaseInsensitive) || strncmp(head.constData(), "PK\x03\x04", 4) == 0) {
        m_pkPassMgr->importPass(url);
    } else {
        m_resMgr->importReservation(f.readAll(), f.fileName());
    }
}

void ApplicationController::importData(const QByteArray &data)
{
    qCDebug(Log);
    if (data.size() < 4) {
        return;
    }
    if (strncmp(data.constData(), "PK\x03\x04", 4) == 0) {
        m_pkPassMgr->importPassFromData(data);
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
