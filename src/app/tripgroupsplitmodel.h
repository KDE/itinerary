/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TRIPGROUPSPLITMODEL_H
#define TRIPGROUPSPLITMODEL_H

#include <QAbstractListModel>
#include <QStringList>

class ReservationManager;

/** Model backing the trip group splitting UI. */
class TripGroupSplitModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(ReservationManager* reservationManager MEMBER m_resMgr NOTIFY reservationManagerChanged REQUIRED)
    Q_PROPERTY(QStringList elements MEMBER m_elements WRITE setElements NOTIFY elementsChanged)
    Q_PROPERTY(QStringList selection READ selection NOTIFY selectionChanged)
public:
    explicit TripGroupSplitModel(QObject *parent = nullptr);
    ~TripGroupSplitModel() override;

    enum Role {
        TitleRole = Qt::DisplayRole,
        SubtitleRole = Qt::UserRole,
        IconNameRole,
        SelectedRole,
    };

    /** All elements in the trip group to split, sorted in time order. */
    void setElements(const QStringList &elements);

    /** The selected elements to split out. */
    [[nodiscard]] QStringList selection() const;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;

Q_SIGNALS:
    void elementsChanged();
    void reservationManagerChanged();
    void selectionChanged();

private:
    ReservationManager *m_resMgr = nullptr;
    QStringList m_elements;
    qsizetype m_beginSelection = 0;
    qsizetype m_endSelection = 0;
};

#endif // TRIPGROUPPROXYMODEL_H
