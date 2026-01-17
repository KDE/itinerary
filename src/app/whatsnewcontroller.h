/*
    SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WHATSNEWCONTROLLER_H
#define WHATSNEWCONTROLLER_H

#include <KAboutData>

#include <QObject>
#include <qqmlregistration.h>

/** Implementation details for the What's new? dialog. */
class WhatsNewController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QList<KAboutRelease> allReleases MEMBER m_allReleases WRITE setAllReleases NOTIFY releasesChanged)
    Q_PROPERTY(QList<KAboutRelease> newReleases READ newReleases NOTIFY releasesChanged)

    Q_PROPERTY(bool hasReleases READ hasReleases NOTIFY releasesChanged)
    Q_PROPERTY(bool hasNewReleases READ hasNewReleases NOTIFY hasNewReleasesChanged)

public:
    explicit WhatsNewController(QObject *parent = nullptr);
    ~WhatsNewController();

    void setAllReleases(const QList<KAboutRelease> &allReleases);
    [[nodiscard]] QList<KAboutRelease> newReleases() const;

    [[nodiscard]] bool hasReleases() const;
    [[nodiscard]] bool hasNewReleases() const;

    Q_INVOKABLE void updateLastSeenVersion();

Q_SIGNALS:
    void lastSeenVersionChanged();
    void releasesChanged();
    void hasNewReleasesChanged();

private:
    QString m_lastSeenVersion;
    QList<KAboutRelease> m_allReleases;
    QList<KAboutRelease> m_newReleases;
};

#endif
