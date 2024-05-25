/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef IMPORTCONTROLLER_H
#define IMPORTCONTROLLER_H

#include <KItinerary/CreativeWork>

#include <KCalendarCore/Calendar>

#include <QAbstractListModel>
#include <QString>
#include <QVariant>

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

class ReservationManager;

namespace KItinerary {
class ExtractorDocumentNode;
class File;
}

namespace KMime {
class Message;
}

namespace KAndroidExtras {
class Intent;
}

class QNetworkAccessManager;
class QUrl;

/** A single importable element. */
class ImportElement
{
public:
    enum Type {
        Reservation,
        Pass,
        HealthCertificate,
        Template,
        Backup,
    };

    Type type;
    QVariant data;
    QVariant updateData = {}; // incremental reservation update, the existing one is in data in that case
    QString id = {};
    QList<QVariant> batch = {};
    int bundleIdx = -1;
    bool selected = true;
};

/** A staged document to import. */
class ImportDocument
{
public:
    QVariant metaData;
    QByteArray data;
};

/** A staged Apple Wallet pass. */
class ImportPkPass
{
public:
    QByteArray data;
};

/** A staged KItinerary bundle file. */
class ImportBundle
{
public:
    std::unique_ptr<QIODevice> backingData;
    std::unique_ptr<KItinerary::File> data;
};

/** Manages the data import staging area.
 *  That is, data parsed/extracted but not yet added to the proper application
 *  state. In that state data can be shown to the user for approval, and, if necessary,
 *  for selecting a trip group to import into.
 *  The model interface provides a view on the current staging area.
 */
class ImportController : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY rowCountChanged)
    Q_PROPERTY(bool enableAutoCommit MEMBER m_autoCommitEnabled WRITE setAutoCommitEnablted NOTIFY enableAutoCommitChanged)
public:
    explicit ImportController(QObject *parent = nullptr);
    ~ImportController();

    void setNetworkAccessManagerFactory(const std::function<QNetworkAccessManager*()> &namFactory);
    void setReservationManager(const ReservationManager *resMgr);

    enum Role {
        TitleRole = Qt::DisplayRole,
        SubtitleRole = Qt::UserRole,
        IconNameRole,
        TypeRole,
        SelectedRole,
        DataRole,
        BatchSizeRole,
        AttachmentCountRole,
    };

    /** Import whatever @p url points to. */
    Q_INVOKABLE void importFromUrl(const QUrl &url);
    /** Import the current clipboard content. */
    Q_INVOKABLE void importFromClipboard();
    /** Import raw data. */
    Q_INVOKABLE void importData(const QByteArray &data, const QString &fileName = {});
    Q_INVOKABLE void importText(const QString &text);
    /** Import from Android Intents. */
    void importFromIntent(const KAndroidExtras::Intent &intent);
    /** Import from a system calendar. */
    Q_INVOKABLE void importFromCalendar(KCalendarCore::Calendar *calendar);

    /** Discard all currently staged content. */
    Q_INVOKABLE void clear();
    /** Discard all selected elements, assuming it has been imported. */
    void clearSelected();

    /** Returns whether the current staged content is allowing auto-committing. */
    Q_INVOKABLE bool canAutoCommit() const;

    // model interface
    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    // properties
    bool hasSelection() const;
    void setAutoCommitEnablted(bool enabled);

    // for actually importing
    const std::vector<ImportElement>& elements() const;
    std::unordered_map<QString, ImportDocument>& documents();
    std::unordered_map<QString, ImportPkPass>& pkPasses();
    const std::vector<ImportBundle>& bundles() const;

Q_SIGNALS:
    /** Emitted when staged elements are added for the first time. */
    void showImportPage();

    /** Passive notification messages, e.g. for errors. */
    void infoMessage(const QString &msg);

    void selectionChanged();
    void rowCountChanged();
    void enableAutoCommitChanged();

private:
    void importLocalFile(const QUrl &url);
    bool importBundle(const QUrl &url);
    bool importBundle(const QByteArray &data);
    bool importBundle(ImportBundle &&bundle);
    void importNode(const KItinerary::ExtractorDocumentNode &node);

    void addElement(ImportElement &&elem);

    [[nodiscard]] QDate today() const;

    std::function<QNetworkAccessManager*()> m_namFactory;
    const ReservationManager *m_resMgr = nullptr;

    std::vector<ImportElement> m_stagedElements;
    std::unordered_map<QString, ImportDocument> m_stagedDocuments;
    std::unordered_map<QString, ImportPkPass> m_stagedPkPasses;
    std::vector<ImportBundle> m_stagedBundles;

    bool m_autoCommitEnabled = false;

    friend class ImportControllerTest;
    QDate m_todayOverride;
};

#endif // IMPORTCONTROLLER_H
