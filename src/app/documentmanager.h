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

#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include <QObject>

/** Manages documents attached to reservations. */
class DocumentManager : public QObject
{
    Q_OBJECT
public:
    explicit DocumentManager(QObject *parent = nullptr);
    ~DocumentManager();

    /** Returns the document meta data. */
    QVariant documentInfo(const QString &id) const;
    /** Returns a file path for opening the document. */
    QString documentFilePath(const QString &id) const;

    /** Add a document from raw data. */
    void addDocument(const QString &id, const QVariant &info, const QByteArray &data);
    /** Add a document from an external file. */
    void addDocument(const QString &id, const QVariant &info, const QString &filePath);
    /** Remove a document. */
    Q_INVOKABLE void removeDocument(const QString &id);

Q_SIGNALS:
    void documentAdded(const QString &id);
    void documentRemoved(const QString &id);

private:
    QString basePath() const;
};

#endif // DOCUMENTMANAGER_H
