/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PASSMANAGER_H
#define PASSMANAGER_H

#include <QAbstractListModel>

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

    enum {
        PassRole = Qt::UserRole,
        PassIdRole,
        PassTypeRole,
    };

    enum PassType {
        ProgramMembership,
        PkPass
    };
    Q_ENUM(PassType)

    bool import(const QVariant &pass);
    bool import(const QVector<QVariant> &passes);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE bool remove(const QString &passId);

    Q_INVOKABLE bool removeRow(int row, const QModelIndex &parent = QModelIndex()); // not exported to QML in Qt5 yet
    Q_INVOKABLE bool removeRows(int row, int count, const QModelIndex &parent = {}) override;

private:
    struct Entry {
        QString id;
        QVariant data;
    };
    mutable std::vector<Entry> m_entries;

    void load();
    void ensureLoaded(Entry &entry) const;
    static QString basePath();
};

#endif // PASSMANAGER_H
