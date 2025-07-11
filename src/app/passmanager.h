/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PASSMANAGER_H
#define PASSMANAGER_H

#include <QAbstractListModel>
#include <QDateTime>

namespace KItinerary
{
class ExtractorValidator;
}

/** Holds time-less pass or program membership elements.
 *  Not to be confused with PkPassManager, which handles storage
 *  and updates of Apple Wallet pass files.
 */
class PassManager : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit PassManager(QObject *parent = nullptr);
    ~PassManager();

    enum Roles {
        PassRole = Qt::UserRole,
        PassIdRole,
        PassTypeRole,
        PassDataRole,
        NameRole,
        ValidUntilRole,
        StateRole,
        ValidRangeLabelRole,
    };
    Q_ENUM(Roles)

    enum PassType {
        ProgramMembership,
        PkPass,
        Ticket,
    };
    Q_ENUM(PassType)

    Q_INVOKABLE QString import(const QVariant &pass, const QString &id = {});
    QStringList import(const QList<QVariant> &passes);

    /** Returns the pass id for the pass that is the closest match
     *  to the given ProgramMembership object.
     *  This is useful for finding the membership pass based on potentially
     *  incomplete data from a ticket.
     *  An empty string is returned if no matching membership is found.
     */
    Q_INVOKABLE [[nodiscard]] QString findMatchingPass(const QVariant &pass) const;
    /** Returns the pass object for @p passId. */
    Q_INVOKABLE [[nodiscard]] QVariant pass(const QString &passId) const;
    /** Returns the raw pass data for @p passId, for exporting. */
    [[nodiscard]] QByteArray passData(const QString &passId) const;

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void update(const QString &passId, const QVariant &pass);
    Q_INVOKABLE bool remove(const QString &passId);

    Q_INVOKABLE bool removeRow(int row, const QModelIndex &parent = QModelIndex()); // not exported to QML in Qt5 yet
    Q_INVOKABLE bool removeRows(int row, int count, const QModelIndex &parent = {}) override;

    /** Ids of documents attached to @p pass. */
    Q_INVOKABLE static QVariantList documentIds(const QVariant &pass);

    /** Validator accepting all supported pass types. */
    [[nodiscard]] static KItinerary::ExtractorValidator validator();

Q_SIGNALS:
    void passChanged(const QString &passId);

private:
    struct Entry {
        QString id;
        QVariant data;

        [[nodiscard]] QString name() const;
        [[nodiscard]] QDateTime validFrom() const;
        [[nodiscard]] QDateTime validUntil() const;
    };

    struct PassComparator {
        PassComparator(const QDateTime &baseTime)
            : m_baseTime(baseTime)
        {
        }
        bool operator()(const PassManager::Entry &lhs, const PassManager::Entry &rhs) const;
        QDateTime m_baseTime;
    };

    void load();
    bool write(const QVariant &data, const QString &id) const;
    [[nodiscard]] QByteArray rawData(const Entry &entry) const;

    static QString basePath();

    std::vector<Entry> m_entries;
    QDateTime m_baseTime;
};

#endif // PASSMANAGER_H
