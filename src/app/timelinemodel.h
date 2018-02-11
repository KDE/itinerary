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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TIMELINEMODEL_H
#define TIMELINEMODEL_H

#include <QAbstractListModel>

class PkPassManager;

class TimelineModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role {
        PassRole = Qt::UserRole + 1,
        PassIdRole,
        SectionHeader
    };
    explicit TimelineModel(QObject *parent = nullptr);
    ~TimelineModel();

    void setPkPassManager(PkPassManager *mgr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void passAdded(const QString &passId);

    PkPassManager *m_mgr = nullptr;
    QVector<QString> m_passes;
};

#endif // TIMELINEMODEL_H
