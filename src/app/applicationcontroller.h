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

#ifndef APPLICATIONCONTROLLER_H
#define APPLICATIONCONTROLLER_H

#include <QObject>

#ifdef Q_OS_ANDROID
#include <QAndroidActivityResultReceiver>
#endif

class PkPassManager;
class ReservationManager;

namespace KItinerary {
class File;
}

class QNetworkAccessManager;

class ApplicationController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasClipboardContent READ hasClipboardContent NOTIFY clipboardContentChanged)
public:
    explicit ApplicationController(QObject *parent = nullptr);
    ~ApplicationController();

    void setReservationManager(ReservationManager *resMgr);
    void setPkPassManager(PkPassManager *pkPassMgr);

    // data import
    Q_INVOKABLE void showImportFileDialog();
    Q_INVOKABLE void importFromClipboard();
    Q_INVOKABLE void importFromUrl(const QUrl &url);
    void importData(const QByteArray &data);

    Q_INVOKABLE void checkCalendar();

    static ApplicationController* instance();

    bool hasClipboardContent() const;

    void importBundle(const QUrl &url);
    void importBundle(const QByteArray &data);

    // data export
    Q_INVOKABLE void exportData();
    void exportToFile(const QString &filePath);

    // for internal use
#ifdef Q_OS_ANDROID
    void importFromIntent(const QAndroidJniObject &intent);
    void exportToIntent(const QAndroidJniObject &intent);
#endif
Q_SIGNALS:
    void clipboardContentChanged();

private:
    void importLocalFile(const QUrl &url);
    void importBundle(KItinerary::File *file);

    static ApplicationController *s_instance;

    ReservationManager *m_resMgr = nullptr;
    PkPassManager *m_pkPassMgr = nullptr;
    QNetworkAccessManager *m_nam = nullptr;
};

#endif // APPLICATIONCONTROLLER_H
