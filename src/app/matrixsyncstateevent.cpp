/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixsyncstateevent.h"

#if HAVE_MATRIX
#include <Quotient/events/stateevent.h>
#endif

#include <QFile>
#include <QSysInfo>
#include <QTemporaryFile>

using namespace Qt::Literals;

constexpr inline auto FORMAT_VERSION = 1;
constexpr inline auto CONTENT_SIZE_LIMIT = 49152;

MatrixSyncStateEvent::MatrixSyncStateEvent() = default;

MatrixSyncStateEvent::MatrixSyncStateEvent(QLatin1StringView type, QString stateKey)
    : m_type(type)
    , m_stateKey(std::move(stateKey))
{
    m_content.insert("version"_L1, FORMAT_VERSION);

    // for easier debugging: indicate which instance wrote this state event
    if (const auto hostName = QSysInfo::machineHostName(); !hostName.isEmpty()) {
        m_content.insert("source"_L1, hostName);
    }
}

MatrixSyncStateEvent::MatrixSyncStateEvent(MatrixSyncStateEvent &&) noexcept = default;
MatrixSyncStateEvent::~MatrixSyncStateEvent() = default;
MatrixSyncStateEvent& MatrixSyncStateEvent::operator=(MatrixSyncStateEvent &&) noexcept = default;

QString MatrixSyncStateEvent::type() const
{
    return m_type;
}

QString MatrixSyncStateEvent::stateKey() const
{
    return m_stateKey;
}

#if HAVE_MATRIX
bool MatrixSyncStateEvent::needsDownload() const
{
    return !url().isEmpty() && m_fileName.isEmpty();
}

bool MatrixSyncStateEvent::needsUpload() const
{
    return url().isEmpty() && (!m_fileName.isEmpty() || m_tempFile);
}

QUrl MatrixSyncStateEvent::url() const
{
    return Quotient::getUrlFromSourceInfo(encryptionData());
}

Quotient::EncryptedFileMetadata MatrixSyncStateEvent::encryptionData() const
{
    Quotient::EncryptedFileMetadata encryptionData;
    Quotient::JsonObjectConverter<Quotient::EncryptedFileMetadata>::fillFrom(m_content.value("file"_L1).toObject(), encryptionData);
    return encryptionData;
}

void MatrixSyncStateEvent::setFileInfo(const Quotient::FileSourceInfo &info)
{
    QJsonObject file;
    Quotient::JsonObjectConverter<Quotient::EncryptedFileMetadata>::dumpTo(file, std::get<Quotient::EncryptedFileMetadata>(info));
    m_content.insert("file"_L1, file);
}
#endif

QString MatrixSyncStateEvent::fileName() const
{
    if (m_tempFile) {
        return m_tempFile->fileName();
    }

    if (!m_fileName.isEmpty()) {
        return m_fileName;
    }

    if (m_content.contains("content"_L1)) { // deal with small file inline storage
        m_tempFile = std::make_unique<QTemporaryFile>();
        m_tempFile->open();
        m_tempFile->write(content());
        m_tempFile->close();
        return m_tempFile->fileName();
    }

    return {};
}

void MatrixSyncStateEvent::setFileName(const QString &fileName)
{
    // download
    if (!url().isEmpty()) {
        m_fileName = fileName;
        return;
    }

    // upload: store small files inline
    QFile f(fileName);
    if (f.open(QFile::ReadOnly) && f.size() < (CONTENT_SIZE_LIMIT * 4 / 3)) {
        setContent(f.readAll());
        return;
    }
    m_fileName = fileName;
}

QByteArray MatrixSyncStateEvent::content() const
{
    switch (contentType()) {
        case None:
            return m_content.value("content"_L1).toString().toUtf8();
        case Base64:
            return QByteArray::fromBase64(m_content.value("content"_L1).toString().toUtf8());
        case File:
        {
            QFile f(m_fileName);
            if (!f.open(QFile::ReadOnly)) {
                qDebug() << f.errorString() << f.fileName();
            }
            return f.readAll();
        }
    }
    return {};
}

void MatrixSyncStateEvent::setContent(const QByteArray &content)
{
    // look for characters that would need some form of quoting in JSON (and thus increase the size)
    qsizetype quotingCost = 0;
    if (content.size() < CONTENT_SIZE_LIMIT) {
        for (const auto c : content) {
            if (c == '\n' || c == '"') {
                ++quotingCost;
            } else if (c < 20 || c >= 127) {
                quotingCost += 3;
            }
        }
    }

    const auto jsonSize = content.size() + quotingCost;
    const auto base64Size = content.size() * 4 / 3;

    if (std::min(jsonSize, base64Size) > CONTENT_SIZE_LIMIT) {
        m_tempFile = std::make_unique<QTemporaryFile>();
        if (!m_tempFile->open()) {
            qWarning() << m_tempFile->errorString() << m_tempFile->fileName();
        }
        m_tempFile->write(content);
        m_tempFile->close();
        m_content.insert("contentType"_L1, "file"_L1);
        return;
    }

    if (jsonSize < base64Size) {
        m_content.insert("content"_L1, QString::fromUtf8(content));
        m_content.insert("contentType"_L1, "none"_L1);
    } else {
        m_content.insert("content"_L1, QString::fromLatin1(content.toBase64()));
        m_content.insert("contentType"_L1, "base64"_L1);
    }
}

QString MatrixSyncStateEvent::roomId() const
{
    return m_roomId;
}

QJsonValue MatrixSyncStateEvent::extraData(QLatin1StringView key) const
{
    return m_content.value("extra"_L1).toObject().value(key);
}

void MatrixSyncStateEvent::setExtraData(QLatin1StringView key, const QJsonValue &value)
{
    auto extras = m_content.value("extra"_L1).toObject();
    extras.insert(key, value);
    m_content.insert("extra"_L1, extras);
}

#if HAVE_MATRIX
Quotient::StateEvent MatrixSyncStateEvent::toQuotient() const
{
    // FIXME can we have a QString overload in Quotient for this?
    return Quotient::StateEvent(QLatin1StringView(m_type.toLatin1()), m_stateKey, m_content);
}

std::optional<MatrixSyncStateEvent> MatrixSyncStateEvent::fromQuotient(const Quotient::StateEvent &state, const QString &roomId)
{
    bool found = false;
    for (const auto t : {MatrixSync::ReservationEventType, MatrixSync::LiveDataEventType, MatrixSync::TransferEventType, MatrixSync::DocumentEventType, MatrixSync::PkPassEventType}) {
        found = state.matrixType() == t;
        if (found) {
            break;
        }
    }
    if (!found) {
        return {};
    }

    if (const auto version = state.contentJson().value("version"_L1).toInt(std::numeric_limits<int>::max()); version >FORMAT_VERSION) {
        qDebug() << "Got state event of unsupported version" << version;
        return {};
    }

    MatrixSyncStateEvent ev;
    ev.m_type = state.matrixType();
    ev.m_stateKey = state.stateKey();
    ev.m_content = state.contentJson();
    ev.m_roomId = roomId; // NOTE: state.roomId() isn't good enough here, that's empty for incoming events from Quotient

    return ev;
}
#endif

MatrixSyncStateEvent::ContentType MatrixSyncStateEvent::contentType() const
{
    const auto ct = m_content.value("contentType"_L1).toString();
    if (ct == "none"_L1) {
        return None;
    }
    if (ct == "base64"_L1) {
        return Base64;
    }
    if (ct == "file"_L1) {
        return File;
    }

    qDebug() << "Unknown state event content type:" << ct;
    return None;
}
