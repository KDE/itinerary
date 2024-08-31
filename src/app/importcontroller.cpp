/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-itinerary.h"

#include "importcontroller.h"

#include "bundle-constants.h"
#include "downloadjob.h"
#include "filehelper.h"
#include "genericpkpass.h"
#include "healthcertificatemanager.h"
#include "logging.h"
#include "passmanager.h"
#include "reservationhelper.h"
#include "reservationmanager.h"
#include "tripgroup.h"

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

#include <KItinerary/DocumentUtil>
#include <KItinerary/Event>
#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/ExtractorValidator>
#include <KItinerary/File>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/LocationUtil>
#include <KItinerary/MergeUtil>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>
#include <KItinerary/Ticket>

#include <KPublicTransport/Line>

#if HAVE_KHEALTHCERTIFICATE
#include <KHealthCertificate/KHealthCertificateParser>
#include <KHealthCertificate/KRecoveryCertificate>
#include <KHealthCertificate/KTestCertificate>
#include <KHealthCertificate/KVaccinationCertificate>
#endif

#include <KMime/Message>

#include <KLocalizedString>

#include <QBuffer>
#include <QClipboard>
#include <QDateTime>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QMimeDatabase>
#include <QUrl>

using namespace Qt::Literals::StringLiterals;

[[nodiscard]] static bool probablyUrl(QStringView text)
{
    return (text.startsWith("https://"_L1) || text.startsWith("http://"_L1)) && text.size() < 256;
}

[[nodiscard]] static KItinerary::EventReservation promoteToReservation(const QVariant &ev)
{
    KItinerary::EventReservation res;
    res.setReservationFor(ev);
    res.setPotentialAction(ev.value<KItinerary::Event>().potentialAction());
    return res;
}

ImportController::ImportController(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &QAbstractItemModel::rowsInserted, this, &ImportController::rowCountChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &ImportController::rowCountChanged);
    connect(this, &QAbstractItemModel::modelReset, this, &ImportController::rowCountChanged);
}

ImportController::~ImportController() = default;

void ImportController::setNetworkAccessManagerFactory(const std::function<QNetworkAccessManager*()> &namFactory)
{
    m_namFactory = namFactory;
}

void ImportController::setReservationManager(const ReservationManager *resMgr)
{
    m_resMgr = resMgr;
}

void ImportController::importFromUrl(const QUrl &url)
{
    qCInfo(Log) << url;
    if (!url.isValid()) {
        return;
    }

    if (FileHelper::isLocalFile(url)) {
        importLocalFile(url);
        return;
    }

    if (url.scheme().startsWith("http"_L1)) {
        auto job = new DownloadJob(url, m_namFactory(), this);
        connect(job, &DownloadJob::finished, this, [this, job]() {
            job->deleteLater();
            if (job->hasError()) {
                Q_EMIT infoMessage(job->errorMessage());
                return;
            }
            importData(job->data());
        });
        return;
    }

    qCWarning(Log) << "Unhandled URL type:" << url;
}

void ImportController::importLocalFile(const QUrl &url)
{
    qCInfo(Log) << url;
    if (url.isEmpty()) {
        return;
    }

    QFile f(FileHelper::toLocalFile(url));
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to open" << f.fileName() << f.errorString();
        Q_EMIT infoMessage(i18n("Import failed: %1", f.errorString()));
        return;
    }
    if (f.size() > 10000000 && !FileHelper::fileName(url).endsWith(".itinerary"_L1)) {
        qCWarning(Log) << "File too large, ignoring" << f.fileName() << f.size();
        Q_EMIT infoMessage(i18n("Import failed: File too large."));
        return;
    }

    // deal with things we can import more efficiently from a file directly
    const auto head = f.peek(4);
    if (FileHelper::hasZipHeader(head)) {
        if (url.fileName().endsWith(".itinerary"_L1, Qt::CaseInsensitive) && importBundle(url)) {
            return;
        }
    }

    QString fileName;
#ifdef Q_OS_ANDROID
    if (url.scheme() == "content"_L1) {
        fileName = KAndroidExtras::ContentResolver::fileName(url);
    }
#endif
    if (fileName.isEmpty()) {
        fileName = f.fileName();
    }

    importData(f.readAll(), fileName);
}

void ImportController::importFromClipboard()
{
    const auto md = QGuiApplication::clipboard()->mimeData();
    if (md->hasUrls()) {
        const auto urls = md->urls();
        for (const auto &url : urls) {
            importFromUrl(url);
        }
    }

    else if (md->hasText()) {
        const auto content = md->data("text/plain"_L1);

        const QString contentString = QString::fromUtf8(content);
        // URL copied as plain text
        if (probablyUrl(contentString)) {
            const QUrl url(contentString);
            if (url.isValid()) {
                importFromUrl(url);
                return;
            }
        }
        importData(content);
    }

    else if (md->hasFormat("application/octet-stream"_L1)) {
        importData(md->data("application/octet-stream"_L1));
    }
}

void ImportController::importData(const QByteArray &data, const QString &fileName)
{
    qCInfo(Log) << data.size() << fileName;
    if (data.size() < 4) {
        Q_EMIT infoMessage(i18n("Found nothing to import."));
        return;
    }

    if (FileHelper::hasZipHeader(data)) {
        if (importBundle(data)) {
            return;
        }
    }

    using namespace KItinerary;
    ExtractorEngine engine;
    // user opened the file, so we can be reasonably sure they assume it contains
    // relevant content, so try expensive extraction methods too
    engine.setHints(ExtractorEngine::ExtractFullPageRasterImages);
    engine.setHints(engine.hints() | ExtractorEngine::ExtractGenericIcalEvents);
    engine.setContextDate(QDateTime(QDate::currentDate(), QTime(0, 0)));
    engine.setData(data, fileName);
    const auto extractorResult = JsonLdDocument::fromJson(engine.extract());

    ExtractorPostprocessor postProc;
    postProc.setContextDate(QDateTime(QDate::currentDate(), QTime(0, 0)));
    postProc.process(extractorResult);
    const auto postProcssedResult = postProc.result();

    auto reservationValidator = ReservationManager::validator();
    const auto passValidator = PassManager::validator();
    ExtractorValidator templateValidator;
    templateValidator.setAcceptedTypes<LodgingBusiness, FoodEstablishment, LocalBusiness>();
    templateValidator.setAcceptOnlyCompleteElements(true);

    // check if we have a document we want to attach here
    QString docId;
    QMimeDatabase db;
    DigitalDocument docInfo;
    const auto mt = db.mimeTypeForFileNameAndData(fileName, data);
    if (mt.name() == "application/pdf"_L1 || mt.name() == "message/rfc822"_L1 || mt.name() == "application/mbox"_L1) {
        docInfo.setName(fileName);
        docInfo.setEncodingFormat(mt.name());
        docId = DocumentUtil::idForContent(data);
    }

    for (auto res : postProcssedResult) {
        if (JsonLd::isA<Event>(res)) { // promote Event to EventReservation
            res = promoteToReservation(res);
        }

        // check if (full) reservation, if so add document and add to staging list
        reservationValidator.setAcceptOnlyCompleteElements(true);
        if (reservationValidator.isValidElement(res)) {
            if (!docId.isEmpty()) {
                DocumentUtil::addDocumentId(res, docId);
            }
            qCDebug(Log) << "Found reservation:" << res;
            addElement({ .type = ImportElement::Reservation, .data = res });
            continue;
        }
        // check if this is a partial update for a reservation we already know
        reservationValidator.setAcceptOnlyCompleteElements(false);
        if (reservationValidator.isValidElement(res)) {
            auto existingRes = m_resMgr->isPartialUpdate(res);
            if (!existingRes.isNull()) {
                qCDebug(Log) << "Found partial update for" << existingRes;
                addElement({ .type = ImportElement::Reservation, .data = existingRes, .updateData = res });
                continue;
            } else {
                qCDebug(Log) << "Got partial update but didn't find matching reservation" << res;
            }
        }

        // check if pass, if so attach document and add to staging list
        if (passValidator.isValidElement(res)) {
            if (!docId.isEmpty()) {
                DocumentUtil::addDocumentId(res, docId);
            }
            addElement({ .type = ImportElement::Pass, .data = res });
            continue;
        }

        // check if template
        // this needs to special-case events, as those wont have times as template
        // and thus will be considered invalid by the validator
        if (templateValidator.isValidElement(res) || JsonLd::isA<EventReservation>(res)) {
            addElement({ .type = ImportElement::Template, .data = res });
        }
    }

    // check for health certificates and pkpass files recursively
    importNode(engine.rootDocumentNode());

    // add document if something actually used it
    if (auto it = m_stagedDocuments.find(docId); it != m_stagedDocuments.end()) {
        (*it).second.metaData = docInfo;
        (*it).second.data = data;
    }

    if (m_stagedElements.empty()) {
        Q_EMIT infoMessage(i18n("Found nothing to import."));
    }
}

void ImportController::importText(const QString& text)
{
    importData(text.toUtf8());
}

void ImportController::importFromIntent(const KAndroidExtras::Intent &intent)
{
#ifdef Q_OS_ANDROID
    using namespace KAndroidExtras;
    const auto action = intent.getAction();

    // opening a URL, can be something to import or a shortcut path
    if (action == Intent::ACTION_VIEW) {
        const QUrl url = intent.getData();
        if (url.scheme() != "page"_L1) {
            importFromUrl(intent.getData());
        }
        return;
    }

    // shared content, e.g. URL from the browser or email applications like FairMail
    if (action == Intent::ACTION_SEND || action == Intent::ACTION_SEND_MULTIPLE) {
        const QString type = intent.getType();
        const auto subject = intent.getStringExtra(Intent::EXTRA_SUBJECT);
        const auto from = intent.getStringArrayExtra(Intent::EXTRA_EMAIL);
        const auto text = intent.getStringExtra(Intent::EXTRA_TEXT);
        qCInfo(Log) << action << type << subject << from << text;
        const QStringList attachments = ItineraryActivity().attachmentsForIntent(intent);
        qCInfo(Log) << attachments;

        if (probablyUrl(text)) {
            importFromUrl(QUrl(text));
            return;
        }

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
            msg.contentType()->setMimeType("multipart/mixed");
            auto body = new KMime::Content;
            body->contentType()->setMimeType(type.toUtf8());
            body->setBody(text.toUtf8());
            msg.appendContent(body);
            for (const auto &a : attachments) {
                QUrl attUrl(a);
                auto att = new KMime::Content;
                att->contentType()->setMimeType(ContentResolver::mimeType(attUrl).toUtf8());
                att->contentTransferEncoding()->setEncoding(KMime::Headers::CEbase64);
                att->contentType()->setName(attUrl.fileName(), "utf-8");
                QFile f(a);
                if (!f.open(QFile::ReadOnly)) {
                    qCWarning(Log) << "Failed to open attachement:" << a << f.errorString();
                    continue;
                }
                att->setBody(f.readAll());
                msg.appendContent(att);
            }
        }

        msg.assemble();
        qDebug().noquote() << msg.encodedContent();
        setAutoCommitEnabled(false);
        importData(msg.encodedContent());
        return;
    }

    qCInfo(Log) << "Unhandled intent action:" << action;
#else
    Q_UNUSED(intent);
#endif
}

void ImportController::importFromCalendar(KCalendarCore::Calendar *calendar)
{
    if (calendar->isLoading()) {
        connect(calendar, &KCalendarCore::Calendar::isLoadingChanged, this, [this, calendar]() {
            importFromCalendar(calendar);
        }, Qt::SingleShotConnection);
        return;
    }

    KItinerary::ExtractorEngine extractorEngine;
    extractorEngine.setHints(KItinerary::ExtractorEngine::ExtractGenericIcalEvents);

    auto reservationValidator = ReservationManager::validator();
    reservationValidator.setAcceptOnlyCompleteElements(true);

    auto calEvents = calendar->events(today().addDays(-5), today().addDays(180));
    int count = 0;
    for (const auto &ev : std::as_const(calEvents)) {
        extractorEngine.clear();
        extractorEngine.setContent(QVariant::fromValue(ev), u"internal/event");

        KItinerary::ExtractorPostprocessor postProc;
        postProc.process(KItinerary::JsonLdDocument::fromJson(extractorEngine.extract()));

        const auto res = postProc.result();
        for (auto r : res) {
            bool selected = true;
            if (KItinerary::JsonLd::isA<KItinerary::Event>(r)) {
                r = promoteToReservation(r);
                selected = false;
            }

            if (!reservationValidator.isValidElement(r)) {
                continue;
            }

            addElement({ .type = ImportElement::Reservation, .data = r, .selected = selected });
            ++count;
        }
    }

    if (count == 0) {
        Q_EMIT infoMessage(i18n("No importable events found in this calendar."));
    }
}

bool ImportController::importBundle(const QUrl &url)
{
    auto f = std::make_unique<KItinerary::File>(FileHelper::toLocalFile(url));
    return importBundle({ .backingData = {}, .data = std::move(f) });
}

bool ImportController::importBundle(const QByteArray &data)
{
    auto buffer = std::make_unique<QBuffer>();
    buffer->setData(data);
    buffer->open(QBuffer::ReadOnly);
    auto f = std::make_unique<KItinerary::File>(buffer.get());
    return importBundle({ .backingData = std::move(buffer), .data = std::move(f) });
}

[[nodiscard]] static bool isFullBackup(const KItinerary::File *file)
{
    return file->hasCustomData(BUNDLE_SETTINGS_DOMAIN, u"settings.ini"_s);
}

bool ImportController::importBundle(ImportBundle &&bundle)
{
    if (!bundle.data->open(KItinerary::File::Read)) {
        qCWarning(Log) << "Failed to open bundle file:" << bundle.data->errorString();
        Q_EMIT infoMessage(i18n("Import failed: %1", bundle.data->errorString()));
        return false;
    }

    const auto tgIds = bundle.data->listCustomData(BUNDLE_TRIPGROUP_DOMAIN);
    if (tgIds.size() > 1 || isFullBackup(bundle.data.get())) {
        addElement({ .type = ImportElement::Backup, .data = {}, .bundleIdx = (int)m_stagedBundles.size() });
        m_stagedBundles.push_back(std::move(bundle));
        return true;
    }

    int count = 0;
    const auto resIds = bundle.data->reservations();
    for (const auto &resId : resIds) {
        addElement({ .type = ImportElement::Reservation, .data = bundle.data->reservation(resId), .id = resId, .bundleIdx = (int)m_stagedBundles.size() });
        ++count;
    }

    const auto passIds = bundle.data->listCustomData(BUNDLE_PASS_DOMAIN);
    for (const auto &id : passIds) {
        const auto data = bundle.data->customData(BUNDLE_PASS_DOMAIN, id);
        const auto pass = KItinerary::JsonLdDocument::fromJsonSingular(QJsonDocument::fromJson(data).object());
        addElement({ .type = ImportElement::Pass, .data = pass, .id = id });
        ++count;
    }

#if HAVE_KHEALTHCERTIFICATE
    const auto certIds = bundle.data->listCustomData(BUNDLE_HEALTH_CERTIFICATE_DOMAIN);
    for (const auto &certId : certIds) {
        const auto cert = KHealthCertificateParser::parse(bundle.data->customData(BUNDLE_HEALTH_CERTIFICATE_DOMAIN, certId));
        addElement({ .type = ImportElement::HealthCertificate, .data = cert });
        ++count;
    }
#endif

    const auto docIds = bundle.data->documents();
    for (const auto &docId : docIds) {
        if (auto it = m_stagedDocuments.find(docId); it != m_stagedDocuments.end()) {
            (*it).second.metaData = bundle.data->documentInfo(docId);
            (*it).second.data = bundle.data->documentData(docId);
        }
    }

    const auto pkPassIds = bundle.data->passes();
    for (const auto &passId :pkPassIds) {
        // careful, there are different id mangling schemes used here!
        const auto [passTypeIdentifier, serialNum] = KItinerary::File::decodePassId(passId);
        const auto pkPassId = KItinerary::DocumentUtil::idForPkPass(passTypeIdentifier, serialNum);
        if (auto it = m_stagedPkPasses.find(pkPassId); it != m_stagedPkPasses.end()) {
            (*it).second.data = bundle.data->passData(passId);
        }
    }

    if (tgIds.size() == 1) {
        m_tripGroup = TripGroup::fromJson(QJsonDocument::fromJson(bundle.data->customData(BUNDLE_TRIPGROUP_DOMAIN, tgIds.front())).object());
        if (!m_tripGroup.hasAutomaticName()) {
            m_tripGroupName = m_tripGroup.name();
        }
    }

    if (count) {
        m_stagedBundles.push_back(std::move(bundle));
        return true;
    }
    return false;
}

void ImportController::importNode(const KItinerary::ExtractorDocumentNode &node)
{
    // search bottom-up, as we might find health certificates insides pkpasses for example
    for (const auto &child : node.childNodes()) {
        importNode(child);
    }

    qCDebug(Log) << "checking extractor document node" << node.mimeType();

    // Apple wallet passes
    if (node.mimeType() == "application/vnd.apple.pkpass"_L1) {
        const auto pass = node.content<KPkPass::Pass*>();
        if (!pass || pass->type() == KPkPass::Pass::Coupon || pass->type() == KPkPass::Pass::StoreCard) {
            // no support for displaying those yet
            return;
        }

        // add generic pkpass wrapper if this is a pass that doesn't belong to any other element
        const auto pkPassId = KItinerary::DocumentUtil::idForPkPass(pass->passTypeIdentifier(), pass->serialNumber());
        if (const auto it = m_stagedPkPasses.find(pkPassId); it != m_stagedPkPasses.end()) {
            (*it).second.data = pass->rawData();
        } else {
            GenericPkPass wrapper;
            wrapper.setName(pass->description());
            wrapper.setPkpassPassTypeIdentifier(pass->passTypeIdentifier());
            wrapper.setPkpassSerialNumber(pass->serialNumber());
            wrapper.setValidUntil(pass->expirationDate());
            QVariant v(wrapper);
            KItinerary::DocumentUtil::addDocumentId(v, KItinerary::DocumentUtil::idForPkPass(pass->passTypeIdentifier(), pass->serialNumber()));
            addElement({ .type = ImportElement::Pass, .data = v });
            m_stagedPkPasses[pkPassId] = ImportPkPass{ .data = pass->rawData() };
        }
    }

#if HAVE_KHEALTHCERTIFICATE
    // check for health certificates
    if (node.mimeType() == "text/plain"_L1 || node.mimeType() == "application/octet-stream"_L1) {
        const auto cert = KHealthCertificateParser::parse(node.mimeType() == "text/plain"_L1 ? node.content<QString>().toUtf8() : node.content<QByteArray>());
        if (!cert.isNull()) {
            addElement({ .type = ImportElement::HealthCertificate, .data = cert });
        }
    }
#endif
}

int ImportController::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return (int)m_stagedElements.size();
}

QVariant ImportController::data(const QModelIndex &index, int role) const
{
    using namespace KItinerary;

    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid)) {
        return {};
    }

    const auto &elem = m_stagedElements[index.row()];
    switch (role) {
        case TitleRole:
        {
            switch (elem.type) {
                case ImportElement::Reservation:
                    return ReservationHelper::label(elem.data);
                case ImportElement::Pass:
                    if (JsonLd::isA<Ticket>(elem.data)) {
                        return elem.data.value<Ticket>().name();
                    }
                    if (JsonLd::isA<ProgramMembership>(elem.data)) {
                        return elem.data.value<ProgramMembership>().programName();
                    }
                    if (JsonLd::isA<GenericPkPass>(elem.data)) {
                        return elem.data.value<GenericPkPass>().name();
                    }
                    break;
                case ImportElement::Template:
                    if (JsonLd::isA<EventReservation>(elem.data)) {
                        return LocationUtil::name(elem.data.value<EventReservation>().reservationFor().value<Event>().location());
                    }
                    return LocationUtil::name(elem.data);
                case ImportElement::HealthCertificate:
                    return HealthCertificateManager::displayName(elem.data);
                case ImportElement::Backup:
                    return i18n("Backup");
            }
            break;
        }
        case SubtitleRole:
        {
            switch (elem.type) {
                case ImportElement::Reservation:
                    if (SortUtil::hasStartTime(elem.data)) {
                        return QLocale().toString(SortUtil::startDateTime(elem.data));
                    }
                    return QLocale().toString(SortUtil::startDateTime(elem.data).date());
                case ImportElement::Pass:
                    if (JsonLd::isA<GenericPkPass>(elem.data)) {
                        return QLocale().toString(elem.data.value<GenericPkPass>().validUntil().date());
                    }
                    return QLocale().toString(SortUtil::startDateTime(elem.data).date());
                case ImportElement::Template:
                    return QString();
                case ImportElement::HealthCertificate:
#if HAVE_KHEALTHCERTIFICATE
                    return QLocale().toString(KHealthCertificate::relevantUntil(elem.data).date());
#else
                    break;
#endif
                case ImportElement::Backup:
                    // TODO can we get the ctime of the backup?
                    break;
            }
            break;
        }
        case IconNameRole:
        {
            // use pkpass icon if we have one
            const auto docIds = DocumentUtil::documentIds(m_stagedElements[index.row()].data) + DocumentUtil::documentIds(m_stagedElements[index.row()].updateData);
            for (const auto &docId : docIds) {
                if (const auto it = m_stagedPkPasses.find(docId.toString()); it != m_stagedPkPasses.end() && !(*it).second.data.isEmpty()) {
                    if (!(*it).second.pass) {
                        (*it).second.pass.reset(KPkPass::Pass::fromData((*it).second.data));
                    }
                    if ((*it).second.pass->hasIcon()) {
                        QUrl passId((*it).first);
                        return QString("image://org.kde.pkpass/"_L1 + passId.host() + '/'_L1 +  QString::fromUtf8(QStringView(passId.path()).mid(1).toUtf8().toBase64(QByteArray::Base64UrlEncoding)) + "/icon"_L1);
                    }
                }
            }

            // ... and generic icons otherwise
            switch (elem.type) {
                case ImportElement::Reservation:
                case ImportElement::Pass:
                    return ReservationHelper::defaultIconName(elem.data);
                case ImportElement::Template:
                    if (JsonLd::isA<LodgingBusiness>(elem.data)) {
                        return u"go-home-symbolic"_s;
                    }
                    if (JsonLd::isA<FoodEstablishment>(elem.data) || JsonLd::isA<LocalBusiness>(elem.data)) {
                        return u"qrc:///images/foodestablishment.svg"_s;
                    }
                    return ReservationHelper::defaultIconName(elem.data);
                case ImportElement::HealthCertificate:
                    return u"cross-shape"_s;
                case ImportElement::Backup:
                    return u"backup"_s;
            }
            return u"question"_s;
        }
        case TypeRole:
            return m_stagedElements[index.row()].type;
        case SelectedRole:
            return m_stagedElements[index.row()].selected;
        case DataRole:
            return m_stagedElements[index.row()].data;
        case BatchSizeRole:
            return m_stagedElements[index.row()].batch.size();
        case AttachmentCountRole:
        {
            int count = 0;
            const auto docIds = DocumentUtil::documentIds(m_stagedElements[index.row()].data) + DocumentUtil::documentIds(m_stagedElements[index.row()].updateData);
            for (const auto &docId : docIds) {
                if (const auto it = m_stagedDocuments.find(docId.toString()); it != m_stagedDocuments.end() && !(*it).second.data.isEmpty()) {
                    ++count;
                }
                if (const auto it = m_stagedPkPasses.find(docId.toString()); it != m_stagedPkPasses.end() && !(*it).second.data.isEmpty()) {
                    ++count;
                }
            }
            return count;
        }
    }

    return {};
}

bool ImportController::setData(const QModelIndex &index, const QVariant &value, int role)
{
    auto &ev = m_stagedElements[index.row()];
    switch (role) {
        case SelectedRole:
            ev.selected = value.toBool();
            Q_EMIT dataChanged(index, index);
            Q_EMIT selectionChanged();
            return true;
    }
    return false;
}

QHash<int, QByteArray> ImportController::roleNames() const
{
    auto n = QAbstractListModel::roleNames();
    n.insert(TitleRole, "title");
    n.insert(SubtitleRole, "subtitle");
    n.insert(IconNameRole, "iconName");
    n.insert(TypeRole, "type");
    n.insert(SelectedRole, "selected");
    n.insert(DataRole, "data");
    n.insert(BatchSizeRole, "batchSize");
    n.insert(AttachmentCountRole, "attachmentCount");
    return n;
}

bool ImportController::hasSelection() const
{
    return std::any_of(m_stagedElements.begin(), m_stagedElements.end(), [](const auto &elem) { return elem.selected; });
}

bool ImportController::hasSelectedReservation() const
{
    return std::any_of(m_stagedElements.begin(), m_stagedElements.end(), [](const auto &elem) {
        return elem.selected && elem.type == ImportElement::Reservation;
    });
}

QVariantList ImportController::selectedReservations() const
{
    QVariantList reservations;
    for (const auto &elem : m_stagedElements) {
        if (elem.selected && elem.type == ImportElement::Reservation) {
            reservations.push_back(elem.data);
        }
    }
    return reservations;
}

QDateTime ImportController::selectionBeginDateTime() const
{
    QDateTime begin;
    for (const auto &elem : m_stagedElements) {
        if (elem.selected && elem.type == ImportElement::Reservation) {
            begin = begin.isValid() ? std::min(begin, KItinerary::SortUtil::startDateTime(elem.data)) : KItinerary::SortUtil::startDateTime(elem.data);
        }
    }
    return begin;
}

QDateTime ImportController::selectionEndDateTime() const
{
    QDateTime end;
    for (const auto &elem : m_stagedElements) {
        if (elem.selected && elem.type == ImportElement::Reservation) {
            end = end.isValid() ? std::max(end, KItinerary::SortUtil::endDateTime(elem.data)) : KItinerary::SortUtil::endDateTime(elem.data);
        }
    }
    return end.isValid() ? end :selectionBeginDateTime();
}

void ImportController::setAutoCommitEnabled(bool enable)
{
    if (m_autoCommitEnabled == enable) {
        return;
    }

    m_autoCommitEnabled = enable;
    Q_EMIT enableAutoCommitChanged();
}

QString ImportController::tripGroupName() const
{
    return m_tripGroupName;
}

void ImportController::setTripGroupName(const QString &tripGroupName)
{
    m_tripGroupName = tripGroupName;
}

QString ImportController::tripGroupId() const
{
    return m_tripGroupId;
}

void ImportController::setTripGroupId(const QString &tripGroupId)
{
    m_tripGroupId = tripGroupId;
}

TripGroup ImportController::tripGroup() const
{
    return m_tripGroup;
}

void ImportController::clear()
{
    beginResetModel();
    m_stagedElements.clear();
    m_stagedDocuments.clear();
    m_stagedPkPasses.clear();
    m_stagedBundles.clear();
    m_tripGroupName.clear();
    m_tripGroupId.clear();
    m_tripGroup = TripGroup();
    m_autoCommitEnabled = false;
    endResetModel();
}

void ImportController::clearSelected()
{
    for (auto i = 0; i < rowCount();) {
        if (!m_stagedElements[i].selected) {
            ++i;
            continue;
        }

        beginRemoveRows({}, i, i);
        m_stagedElements.erase(m_stagedElements.begin() + i);
        endRemoveRows();
    }

    m_tripGroupName.clear();
    m_tripGroupId.clear();
    m_tripGroup = TripGroup();
}

bool ImportController::canAutoCommit() const
{
    // always auto-commit single templates, as that will result in the better UI
    if (m_stagedElements.size() == 1 && m_stagedElements[0].type == ImportElement::Template) {
        return true;
    }

    if (!m_autoCommitEnabled) {
        return false;
    }

    // everything is selected
    return std::all_of(m_stagedElements.begin(), m_stagedElements.end(), [](const auto &elem) { return elem.selected; });
}

[[nodiscard]] static QDateTime elementSortTime(const ImportElement &elem)
{
    switch (elem.type) {
        case ImportElement::Reservation:
            break; // handled elsewhere
        case ImportElement::Pass:
            if (KItinerary::JsonLd::isA<GenericPkPass>(elem.data)) {
                return elem.data.value<GenericPkPass>().validUntil();
            }
            return KItinerary::SortUtil::startDateTime(elem.data);
        case ImportElement::Template:
            return {};
        case ImportElement::HealthCertificate:
#if HAVE_KHEALTHCERTIFICATE
            return KHealthCertificate::relevantUntil(elem.data);
#else
            break;
#endif
        case ImportElement::Backup:
            break;
    }
    return {};
}

[[nodiscard]] static bool elementLessThan(const ImportElement &lhs, const ImportElement &rhs)
{
    if (lhs.type == ImportElement::Reservation && rhs.type != ImportElement::Reservation) {
        return true;
    }
    if (rhs.type == ImportElement::Reservation && lhs.type != ImportElement::Reservation) {
        return false;
    }

    if (lhs.type != ImportElement::Backup && rhs.type == ImportElement::Backup) {
        return true;
    }
    if (lhs.type == ImportElement::Backup && rhs.type != ImportElement::Backup) {
        return false;
    }

    if (lhs.type == ImportElement::Reservation && rhs.type == ImportElement::Reservation) {
        return KItinerary::SortUtil::isBefore(lhs.data, rhs.data);
    }

    return elementSortTime(lhs) <elementSortTime(rhs);
}

void ImportController::addElement(ImportElement &&elem)
{
    // register all documents and passes we expect
    const auto docIds = KItinerary::DocumentUtil::documentIds(elem.data) + KItinerary::DocumentUtil::documentIds(elem.updateData);
    for (const auto &docId : docIds) {
        if (const QUrl pkPassId(docId.toString()); pkPassId.scheme() == "pkpass"_L1) {
            const auto it = m_stagedPkPasses.find(docId.toString());
            if (it == m_stagedPkPasses.end()) {
                m_stagedPkPasses[docId.toString()] = {};
            }
        } else {
            const auto it = m_stagedDocuments.find(docId.toString());
            if (it == m_stagedDocuments.end()) {
                m_stagedDocuments[docId.toString()] = {};
            }
        }
    }

    // batch multi-traveler reservations
    if (elem.type == ImportElement::Reservation) {
        for (auto &e : m_stagedElements) {
            if (elem.type != e.type) {
                continue;
            }
            if (KItinerary::MergeUtil::isSameIncidence(elem.data, e.data)) {
                e.batch.push_back(elem.data);
                return;
            }
        }
    }

    const auto it = std::lower_bound(m_stagedElements.begin(), m_stagedElements.end(), elem, elementLessThan);
    const auto row = (int)std::distance(m_stagedElements.begin(), it);
    beginInsertRows({}, row, row);
    m_stagedElements.insert(it, std::move(elem));
    endInsertRows();
    Q_EMIT selectionChanged();

    if (m_stagedElements.size() == 1) {
        // delay the emission until we have processed everything that imported
        QMetaObject::invokeMethod(this, &ImportController::showImportPage, Qt::QueuedConnection);
    }
}

const std::vector<ImportElement>& ImportController::elements() const
{
    return m_stagedElements;
}

std::unordered_map<QString, ImportDocument>& ImportController::documents()
{
    return m_stagedDocuments;
}

std::unordered_map<QString, ImportPkPass>& ImportController::pkPasses()
{
    return m_stagedPkPasses;
}

const std::vector<ImportBundle>& ImportController::bundles() const
{
    return m_stagedBundles;
}

QDate ImportController::today() const
{
    if (Q_UNLIKELY(m_todayOverride.isValid())) {
        return m_todayOverride;
    }
    return QDate::currentDate();
}

#include "moc_importcontroller.cpp"
