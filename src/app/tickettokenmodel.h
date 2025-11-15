/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TICKETTOKENMODEL_H
#define TICKETTOKENMODEL_H

#include <QAbstractListModel>
#include <qqmlintegration.h>

class ReservationManager;

/** Filtered model of all reservations with a valid ticket token
 *  for display in the details page.
 */
class TicketTokenModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(ReservationManager *reservationManager READ reservationManager WRITE setReservationManager)
    Q_PROPERTY(QString batchId READ batchId WRITE setBatchId NOTIFY batchIdChanged)
    Q_PROPERTY(int initialIndex READ initialIndex NOTIFY initialIndexChanged)
    Q_PROPERTY(QStringList selectedReservationIds READ selectedReservationIds NOTIFY selectionChanged)

public:
    enum Roles {
        ReservationRole = Qt::UserRole,
        SelectedRole
    };

    explicit TicketTokenModel(QObject *parent = nullptr);
    ~TicketTokenModel() override;

    [[nodiscard]] ReservationManager *reservationManager() const;
    void setReservationManager(ReservationManager *mgr);
    [[nodiscard]] QString batchId() const;
    void setBatchId(const QString &batchId);

    Q_INVOKABLE [[nodiscard]] QVariant reservationAt(int row) const;
    Q_INVOKABLE [[nodiscard]] QString reservationIdAt(int row) const;

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &data, int role) override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int initialIndex() const;
    [[nodiscard]] QStringList selectedReservationIds() const;

Q_SIGNALS:
    void batchIdChanged();
    void initialIndexChanged();
    void selectionChanged();

private:
    void reload();
    void reservationRemoved(const QString &resId);

    ReservationManager *m_resMgr = nullptr;
    QString m_batchId;
    struct Data {
        QString resId;
        QString personName;
        bool selected = false;
    };
    std::vector<Data> m_data;
};

#endif // TICKETTOKENMODEL_H
