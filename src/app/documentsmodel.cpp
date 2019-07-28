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

#include "documentsmodel.h"
#include "documentmanager.h"
#include "reservationmanager.h"

#include <KItinerary/CreativeWork>
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

void DocumentsModel::setDocumentManager(DocumentManager *mgr)
{
    if (m_docMgr == mgr) {
        return;
    }
    m_docMgr = mgr;

    // ### can be done more efficiently!
    connect(mgr, &DocumentManager::documentAdded, this, &DocumentsModel::reload);
    connect(mgr, &DocumentManager::documentRemoved, this, &DocumentsModel::reload);

    emit setupChanged();
}

void DocumentsModel::reload()
{
    if (!m_docMgr || !m_resMgr || m_batchId.isEmpty()) {
        return;
    }

    beginResetModel();
    m_docIds.clear();
    const auto resIds = m_resMgr->reservationsForBatch(m_batchId);
    for (const auto &resId : resIds) {
        const auto res = m_resMgr->reservation(resId);
        if (!JsonLd::canConvert<Reservation>(res)) {
            continue;
        }
        const auto docIds = JsonLd::convert<Reservation>(res).subjectOf();
        for (const auto &docId : docIds) {
            const auto id = docId.toString();
            if (!id.isEmpty() && m_docMgr->hasDocument(id)) {
                m_docIds.push_back(id);
            }
        }
    }

    std::sort(m_docIds.begin(), m_docIds.end());
    m_docIds.erase(std::unique(m_docIds.begin(), m_docIds.end()), m_docIds.end());
    endResetModel();
}

#include "moc_documentsmodel.cpp"
