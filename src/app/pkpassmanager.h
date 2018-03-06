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
    PkPassManager(QObject *parent = nullptr);
    ~PkPassManager();

    QVector<QString> passes() const;

    KPkPass::Pass* pass(const QString &passId);

    Q_INVOKABLE void importPass(const QUrl &url);
    void importPassFromTempFile(const QString &tmpFile);
    Q_INVOKABLE void removePass(const QString &passId);

    void updatePass(const QString &passId);
    Q_INVOKABLE void updatePasses();

signals:
    void passAdded(const QString &passId);
    void passUpdated(const QString &passId, const QStringList &changes);

private:
    enum ImportMode { Copy, Move };
    void doImportPass(const QUrl &url, ImportMode mode);

    QHash<QString, KPkPass::Pass*> m_passes;
    QNetworkAccessManager *m_nam;
};

#endif // PKPASSMANAGER_H
