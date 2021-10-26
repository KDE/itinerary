/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    ~DocumentManager() override;

    /** Lists all documents in storage. */
    QVector<QString> documents() const;
    /** Returns @c true if a document with the given id is present. */
    bool hasDocument(const QString &id) const;

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
