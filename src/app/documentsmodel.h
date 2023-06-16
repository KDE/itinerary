/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DOCUMENTSMODEL_H
#define DOCUMENTSMODEL_H

#include <QAbstractListModel>

#include <vector>

class DocumentManager;

/** Model containing the documents attached to the given batch of reservations. */
class DocumentsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList documentIds MEMBER m_requestedDocIds WRITE setDocumentIds NOTIFY requestedDocumentIdsChanged)
    Q_PROPERTY(DocumentManager* documentManager MEMBER m_docMgr WRITE setDocumentManager NOTIFY setupChanged)
    Q_PROPERTY(bool empty READ isEmpty NOTIFY emptyChanged)

public:
    explicit DocumentsModel(QObject *parent = nullptr);
    ~DocumentsModel() override;

    enum Roles {
        DocumentIdRole = Qt::UserRole,
        DocumentInfoRole,
        DocumentFilePathRole
    };

    int rowCount(const QModelIndex & parent = {}) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setDocumentIds(const QStringList &docIds);
    void setDocumentManager(DocumentManager *mgr);

Q_SIGNALS:
    void requestedDocumentIdsChanged();
    void setupChanged();
    void emptyChanged();

private:
    bool isEmpty() const;
    void reload();

    QStringList m_requestedDocIds;
    std::vector<QString> m_docIds;
    DocumentManager *m_docMgr = nullptr;
};

#endif // DOCUMENTSMODEL_H
