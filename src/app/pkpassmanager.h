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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
    ~PkPassManager();

    QVector<QString> passes() const;

    KPkPass::Pass* pass(const QString &passId);
    Q_INVOKABLE bool hasPass(const QString &passId) const;
    Q_INVOKABLE QObject* passObject(const QString &passId);
    Q_INVOKABLE QString passId(const QVariant &reservation) const;

    /** Import pass from a local @p url, returns the pass id if successful. */
    QString importPass(const QUrl &url);
    void importPassFromData(const QByteArray &data);
    Q_INVOKABLE void removePass(const QString &passId);

    void updatePass(const QString &passId);
    /** Time the pass was last updated (ie. file system mtime). */
    QDateTime updateTime(const QString &passId) const;

    static QDateTime relevantDate(KPkPass::Pass *pass);

Q_SIGNALS:
    void passAdded(const QString &passId);
    void passUpdated(const QString &passId, const QStringList &changes);
    void passRemoved(const QString &passId);

private:
    enum ImportMode { Copy, Move, Data };
    void importPassFromTempFile(const QUrl &tmpFile);
    QString doImportPass(const QUrl &url, const QByteArray &data, ImportMode mode);

    QHash<QString, KPkPass::Pass*> m_passes;
    QNetworkAccessManager *m_nam;
};

#endif // PKPASSMANAGER_H
