/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef APPLICATIONCONTROLLER_H
#define APPLICATIONCONTROLLER_H

#include <QObject>

#ifdef Q_OS_ANDROID
#include <QAndroidActivityResultReceiver>
#endif

class DocumentManager;
class FavoriteLocationModel;
class LiveDataManager;
class PkPassManager;
class ReservationManager;
class TransferManager;

namespace KItinerary {
class File;
}

class QNetworkAccessManager;

class ApplicationController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasClipboardContent READ hasClipboardContent NOTIFY clipboardContentChanged)
    Q_PROPERTY(QString version READ applicationVersion CONSTANT)
    Q_PROPERTY(QString extractorCapabilities READ extractorCapabilities CONSTANT)
public:
    explicit ApplicationController(QObject *parent = nullptr);
    ~ApplicationController();

    void setReservationManager(ReservationManager *resMgr);
    void setPkPassManager(PkPassManager *pkPassMgr);
    void setDocumentManager(DocumentManager *docMgr);
    void setTransferManager(TransferManager *transferMgr);
    void setFavoriteLocationModel(FavoriteLocationModel *favLocModel);
    void setLiveDataManager(LiveDataManager *liveDataMgr);

    // data import
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

    // document attaching
    Q_INVOKABLE void addDocument(const QString &batchId);
    Q_INVOKABLE void removeDocument(const QString &batchId, const QString &docId);
    Q_INVOKABLE void openDocument(const QUrl &url);

    // about information
    QString applicationVersion() const;
    QString extractorCapabilities() const;

Q_SIGNALS:
    void clipboardContentChanged();

private:
    void importLocalFile(const QUrl &url);
    void importBundle(KItinerary::File *file);
    void addDocument(const QString &batchId, const QUrl &url);

    static ApplicationController *s_instance;

    ReservationManager *m_resMgr = nullptr;
    PkPassManager *m_pkPassMgr = nullptr;
    DocumentManager *m_docMgr = nullptr;
    TransferManager *m_transferMgr = nullptr;
    FavoriteLocationModel *m_favLocModel = nullptr;
    LiveDataManager *m_liveDataMgr = nullptr;
    QNetworkAccessManager *m_nam = nullptr;
};

#endif // APPLICATIONCONTROLLER_H
