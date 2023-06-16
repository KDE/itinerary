/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "documentsmodel.h"
#include "documentmanager.h"

#include <KItinerary/CreativeWork>
#include <KItinerary/DocumentUtil>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Reservation>

#include <QDebug>
#include <QMimeDatabase>
#include <QUrl>

using namespace KItinerary;

DocumentsModel::DocumentsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &DocumentsModel::setupChanged, this, &DocumentsModel::reload);
}

DocumentsModel::~DocumentsModel() = default;

int DocumentsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_docIds.size();
}

QVariant DocumentsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_docMgr) {
        return {};
    }

    switch (role) {
        case Qt::DisplayRole:
        {
            const auto info = m_docMgr->documentInfo(m_docIds[index.row()]);
            if (JsonLd::canConvert<CreativeWork>(info)) {
                const auto docInfo = JsonLd::convert<CreativeWork>(info);
                return docInfo.description().isEmpty() ? docInfo.name() : docInfo.description();
            }
            break;
        }
        case Qt::DecorationRole:
        {
            const auto info = m_docMgr->documentInfo(m_docIds[index.row()]);
            if (JsonLd::canConvert<CreativeWork>(info)) {
                const auto docInfo = JsonLd::convert<CreativeWork>(info);
                QMimeDatabase db;
                const auto mt = db.mimeTypeForName(docInfo.encodingFormat());
                return mt.iconName();
            }
            break;
        }
        case DocumentIdRole:
            return m_docIds[index.row()];
        case DocumentInfoRole:
            return m_docMgr->documentInfo(m_docIds[index.row()]);
        case DocumentFilePathRole:
            return QUrl::fromLocalFile(m_docMgr->documentFilePath(m_docIds[index.row()]));
    }

    return {};
}

QHash<int, QByteArray> DocumentsModel::roleNames() const
{
    auto names = QAbstractListModel::roleNames();
    names.insert(DocumentIdRole, "id");
    names.insert(DocumentInfoRole, "info");
    names.insert(DocumentFilePathRole, "filePath");
    return names;
}

void DocumentsModel::setDocumentIds(const QStringList &docIds)
{
    if (m_requestedDocIds == docIds) {
        return;
    }
    m_requestedDocIds = docIds;
    Q_EMIT requestedDocumentIdsChanged();
    reload();
}

void DocumentsModel::setDocumentManager(DocumentManager *mgr)
{
    if (m_docMgr == mgr) {
        return;
    }
    m_docMgr = mgr;

    // ### can be done more efficiently!
    connect(mgr, &DocumentManager::documentAdded, this, &DocumentsModel::reload);
    connect(mgr, &DocumentManager::documentRemoved, this, &DocumentsModel::reload);

    Q_EMIT setupChanged();
}

void DocumentsModel::reload()
{
    if (!m_docMgr) {
        return;
    }

    beginResetModel();
    m_docIds.clear();
    for (const auto &id : m_requestedDocIds) {
        if (!id.isEmpty() && m_docMgr->hasDocument(id)) {
            m_docIds.push_back(id);
        }
    }

    std::sort(m_docIds.begin(), m_docIds.end());
    m_docIds.erase(std::unique(m_docIds.begin(), m_docIds.end()), m_docIds.end());
    endResetModel();
    Q_EMIT emptyChanged();
}

bool DocumentsModel::isEmpty() const
{
    return m_docIds.empty();
}

#include "moc_documentsmodel.cpp"
