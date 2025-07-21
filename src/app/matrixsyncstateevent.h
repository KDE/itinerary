/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MATRIXSYNCSTATEEVENT_H
#define MATRIXSYNCSTATEEVENT_H

#include <config-itinerary.h>

#if HAVE_MATRIX
#include <Quotient/events/filesourceinfo.h>
#endif

#include <QJsonObject>
#include <QString>

#include <memory>

class DocumentManager;
class LiveDataManager;
class ReservationManager;
class TransferManager;

namespace Quotient {
class StateEvent;
}

class QTemporaryFile;

namespace MatrixSync {
    using namespace Qt::Literals;
    constexpr inline auto ReservationEventType = "org.kde.itinerary.reservation"_L1;
    constexpr inline auto LiveDataEventType = "org.kde.itinerary.livedata"_L1;
    constexpr inline auto TransferEventType = "org.kde.itinerary.transfer"_L1;
    constexpr inline auto DocumentEventType = "org.kde.itinerary.document"_L1;
    constexpr inline auto PkPassEventType = "org.kde.itinerary.pkpass"_L1;
}

/** Matrix-based trip synchronization state event wrapper.
 *  Main objective here is abstracting away the event size limit and transparently
 *  put larger payloads into files.
 */
class MatrixSyncStateEvent
{
public:
    // create a new outgoing state event
    explicit MatrixSyncStateEvent(QLatin1StringView type, QString stateKey);
    MatrixSyncStateEvent(MatrixSyncStateEvent &&) noexcept;
    ~MatrixSyncStateEvent();
    MatrixSyncStateEvent& operator=(MatrixSyncStateEvent &&) noexcept;

    [[nodiscard]] QString type() const;
    [[nodiscard]] QString stateKey() const;

    [[nodiscard]] bool needsDownload() const;
    [[nodiscard]] bool needsUpload() const;

    // URL for content stored in an external file
    [[nodiscard]] QUrl url() const;

    // metadata for (encrypted) external files
#if HAVE_MATRIX
    [[nodiscard]] Quotient::EncryptedFileMetadata encryptionData() const;
    void setFileInfo(const Quotient::FileSourceInfo &info);
#endif

    // local file name before upload/after download
    [[nodiscard]] QString fileName() const;
    void setFileName(const QString &fileName);

    // main content, can be stored inline or in an external file
    [[nodiscard]] QByteArray content() const;
    void setContent(const QByteArray &content);

    // Id of the room this event came from (for incoming events only)
    [[nodiscard]] QString roomId() const;

    // extra data, must be very small as this isn't separately protected against exceeding the event size limit
    [[nodiscard]] QJsonValue extraData(QLatin1StringView key) const;
    void setExtraData(QLatin1StringView key, const QJsonValue &value);

    // convert to Quotient state event
    [[nodiscard]] Quotient::StateEvent toQuotient() const;

    // create from an incoming state event
    static std::optional<MatrixSyncStateEvent> fromQuotient(const Quotient::StateEvent &state, const QString &roomId);

private:
    explicit MatrixSyncStateEvent();

    enum ContentType {
        None,
        Base64,
        File
    };
    [[nodiscard]] ContentType contentType() const;

    QString m_type;
    QString m_stateKey;
    QJsonObject m_content;
    QString m_roomId;
    QString m_fileName;
    std::unique_ptr<QTemporaryFile> m_tempFile;
};

#endif
