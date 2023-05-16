/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PKPASSMANAGER_H
#define PKPASSMANAGER_H

#include <QHash>
#include <QObject>

class QNetworkAccessManager;

namespace KPkPass {
class Pass;
}

class PkPassManager : public QObject
{
    Q_OBJECT
public:
    explicit PkPassManager(QObject *parent = nullptr);
    ~PkPassManager() override;

    QVector<QString> passes() const;

    Q_INVOKABLE KPkPass::Pass* pass(const QString &passId);
    Q_INVOKABLE bool hasPass(const QString &passId) const;
    Q_INVOKABLE QString passId(const QVariant &reservation) const;

    /** Import pass from a local @p url, returns the pass id if successful. */
    QString importPass(const QUrl &url);
    QString importPassFromData(const QByteArray &data);
    Q_INVOKABLE void removePass(const QString &passId);

    void updatePass(const QString &passId);
    /** Time the pass was last updated (ie. file system mtime). */
    QDateTime updateTime(const QString &passId) const;

    static QDateTime relevantDate(KPkPass::Pass *pass);

    /** Raw pass file data, used for exporting. */
    QByteArray rawData(const QString &passId) const;

Q_SIGNALS:
    void passAdded(const QString &passId);
    void passUpdated(const QString &passId, const QStringList &changes);
    void passRemoved(const QString &passId);

private:
    enum ImportMode { Copy, Move, Data };
    void importPassFromTempFile(const QUrl &tmpFile);
    QString doImportPass(const QUrl &url, const QByteArray &data, ImportMode mode);

    QNetworkAccessManager *nam();

    QHash<QString, KPkPass::Pass*> m_passes;
    QNetworkAccessManager *m_nam = nullptr;
};

#endif // PKPASSMANAGER_H
