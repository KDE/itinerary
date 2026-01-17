/*
    SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "whatsnewcontroller.h"

#include <QCoreApplication>
#include <QDebug>
#include <QSettings>
#include <QVersionNumber>

WhatsNewController::WhatsNewController(QObject *parent)
    : QObject(parent)
{
    QSettings settings;
    settings.beginGroup("WhatsNew");
    m_lastSeenVersion = settings.value("LastSeenVersion", QCoreApplication::applicationVersion()).toString();

    connect(this, &WhatsNewController::releasesChanged, this, &WhatsNewController::hasNewReleasesChanged);
}

WhatsNewController::~WhatsNewController() = default;

void WhatsNewController::setAllReleases(const QList<KAboutRelease> &allReleases)
{
    m_allReleases = allReleases;
    m_newReleases.clear();

    const auto lastVer = QVersionNumber::fromString(m_lastSeenVersion);
    std::ranges::copy_if(m_allReleases, std::back_inserter(m_newReleases), [lastVer](const KAboutRelease &r) {
        return QVersionNumber::fromString(r.version()) > lastVer;
    });

    Q_EMIT releasesChanged();
}

QList<KAboutRelease> WhatsNewController::newReleases() const
{
    return m_newReleases;
}

bool WhatsNewController::hasReleases() const
{
    return !m_allReleases.empty();
}

bool WhatsNewController::hasNewReleases() const
{
    return !m_newReleases.empty() && QVersionNumber::fromString(m_lastSeenVersion) < QVersionNumber::fromString(QCoreApplication::applicationVersion());
}

void WhatsNewController::updateLastSeenVersion()
{
    // this does intentionally not update m_newReleases, so things don't change in the UI
    m_lastSeenVersion = QCoreApplication::applicationVersion();
    QSettings settings;
    settings.beginGroup("WhatsNew");
    settings.setValue("LastSeenVersion", m_lastSeenVersion);

    Q_EMIT hasNewReleasesChanged();
}

#include "moc_whatsnewcontroller.cpp"
