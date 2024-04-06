/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PKPASSMANAGER_H
#define PKPASSMANAGER_H

#include <QHash>
#include <QObject>

#include <functional>

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

    void setNetworkAccessManagerFactory(const std::function<QNetworkAccessManager*()> &namFactory);

    [[nodiscard]] static QList<QString> passes();

    Q_INVOKABLE [[nodiscard]] KPkPass::Pass* pass(const QString &passId);
    Q_INVOKABLE [[nodiscard]] bool hasPass(const QString &passId) const;
    Q_INVOKABLE [[nodiscard]] static QString passId(const QVariant &reservation);

    /** Import pass from a local @p url, returns the pass id if successful. */
    QString importPass(const QUrl &url);
    QString importPassFromData(const QByteArray &data);
    Q_INVOKABLE void removePass(const QString &passId);

    void updatePass(const QString &passId);
    /** Time the pass was last updated (ie. file system mtime). */
    [[nodiscard]] static QDateTime updateTime(const QString &passId);

    [[nodiscard]] static QDateTime relevantDate(KPkPass::Pass *pass);
    /** Check whether @p pass can be online updated. */
    [[nodiscard]] static bool canUpdate(KPkPass::Pass *pass);

    /** Raw pass file data, used for exporting. */
    [[nodiscard]] static QByteArray rawData(const QString &passId);

Q_SIGNALS:
    void passAdded(const QString &passId);
    void passUpdated(const QString &passId, const QStringList &changes);
    void passRemoved(const QString &passId);

private:
    enum ImportMode { Copy, Move, Data };
    void importPassFromTempFile(const QUrl &tmpFile);
    QString doImportPass(const QUrl &url, const QByteArray &data, ImportMode mode);

    QHash<QString, KPkPass::Pass*> m_passes;
    std::function<QNetworkAccessManager*()> m_namFactory;
};

#endif // PKPASSMANAGER_H
