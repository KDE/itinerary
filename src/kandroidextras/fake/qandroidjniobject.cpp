/*
    SPDX-FileCopyrightText: 2020-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qandroidjniobject.h"

QStringList QAndroidJniObject::m_staticProtocol;

class QAndroidJniObjectPrivate : public QSharedData
{
public:
    QStringList protocol;
};

QAndroidJniObject::QAndroidJniObject()
    : d(new QAndroidJniObjectPrivate)
{
}

QAndroidJniObject::QAndroidJniObject(const char *className)
    : d(new QAndroidJniObjectPrivate)
{
    addToProtocol(QLatin1String("ctor: ") + QLatin1String(className));
}

QAndroidJniObject::QAndroidJniObject(const char* className, const char* signature, ...)
    : d(new QAndroidJniObjectPrivate)
{
    addToProtocol(QLatin1String("ctor: ") + QLatin1String(className) + QLatin1Char(' ') + QLatin1String(signature));
}

QAndroidJniObject::QAndroidJniObject(const QAndroidJniObject&) = default;
QAndroidJniObject & QAndroidJniObject::operator=(const QAndroidJniObject&) = default;
QAndroidJniObject::~QAndroidJniObject() = default;

QStringList QAndroidJniObject::protocol() const
{
    return d->protocol;
}

void QAndroidJniObject::addToProtocol(const QString &line) const
{
    d->protocol.push_back(line);
}

void QAndroidJniObject::setProtocol(const QStringList &protocol)
{
    d->protocol = protocol;
}
