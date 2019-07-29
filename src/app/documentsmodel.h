/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DOCUMENTSMODEL_H
#define DOCUMENTSMODEL_H

#include "reservationmanager.h"

#include <QAbstractListModel>

#include <vector>

class DocumentManager;
class ReservationManager;

/** Model containing the documents attached to the given batch of reservations. */
class DocumentsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString batchId MEMBER m_batchId NOTIFY setupChanged)
    Q_PROPERTY(DocumentManager* documentManager MEMBER m_docMgr WRITE setDocumentManager NOTIFY setupChanged)
    Q_PROPERTY(ReservationManager* reservationManager MEMBER m_resMgr NOTIFY setupChanged)
    Q_PROPERTY(bool empty READ isEmpty NOTIFY emptyChanged)

public:
    explicit DocumentsModel(QObject *parent = nullptr);
    ~DocumentsModel();

    enum Roles {
        DocumentIdRole = Qt::UserRole,
        DocumentInfoRole,
        DocumentFilePathRole
    };

    int rowCount(const QModelIndex & parent) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setDocumentManager(DocumentManager *mgr);

Q_SIGNALS:
    void setupChanged();
    void emptyChanged();

private:
    bool isEmpty() const;
    void reload();

    QString m_batchId;
    std::vector<QString> m_docIds;
    DocumentManager *m_docMgr = nullptr;
    ReservationManager *m_resMgr = nullptr;
};

#endif // DOCUMENTSMODEL_H
