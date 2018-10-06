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

#ifndef APPLICATIONCONTROLLER_H
#define APPLICATIONCONTROLLER_H

#include <QObject>

#ifdef Q_OS_ANDROID
#include <QAndroidActivityResultReceiver>
#endif

class PkPassManager;
class ReservationManager;

class QGeoPositionInfo;
class QGeoPositionInfoSource;

class ApplicationController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasClipboardContent READ hasClipboardContent NOTIFY clipboardContentChanged)
public:
    explicit ApplicationController(QObject *parent = nullptr);
    ~ApplicationController();

    void setReservationManager(ReservationManager *resMgr);
    void setPkPassManager(PkPassManager *pkPassMgr);

    // navigation
    Q_INVOKABLE void showOnMap(const QVariant &place);
    Q_INVOKABLE bool canNavigateTo(const QVariant &place);
    Q_INVOKABLE void navigateTo(const QVariant &place);

    // data import
    Q_INVOKABLE void showImportFileDialog();
    Q_INVOKABLE void importFromClipboard();
#ifdef Q_OS_ANDROID
    void importFromIntent(const QAndroidJniObject &intent);
#endif
    void importLocalFile(const QUrl &url);
    void importData(const QByteArray &data);

    Q_INVOKABLE void checkCalendar();

    static ApplicationController* instance();

    bool hasClipboardContent() const;

signals:
    void clipboardContentChanged();

private:
    static ApplicationController *s_instance;

    ReservationManager *m_resMgr = nullptr;
    PkPassManager *m_pkPassMgr = nullptr;

#ifndef Q_OS_ANDROID
    void navigateTo(const QGeoPositionInfo &from, const QVariant &to);

    QGeoPositionInfoSource *m_positionSource = nullptr;
    QMetaObject::Connection m_pendingNavigation;
#else
    class ActivityResultReceiver : public QAndroidActivityResultReceiver {
    public:
        explicit inline ActivityResultReceiver(ApplicationController *controller)
            : m_controller(controller) {}
        void handleActivityResult(int receiverRequestCode, int resultCode, const QAndroidJniObject &intent) override;
    private:
        ApplicationController *m_controller;
    };
    ActivityResultReceiver m_activityResultReceiver;
#endif
};

#endif // APPLICATIONCONTROLLER_H
