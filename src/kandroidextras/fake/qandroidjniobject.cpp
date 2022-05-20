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
    QHash<QByteArray, QVariant> properties;
    QVariant value;
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

QVariant QAndroidJniObject::property(const QByteArray &name) const
{
    return d->properties.value(name);
}

void QAndroidJniObject::setProperty(const QByteArray &name, const QVariant &value)
{
    d->properties.insert(name, value);
}

QVariant QAndroidJniObject::value() const
{
    return d->value;
}

void QAndroidJniObject::setValue(const QVariant &value)
{
    d->value = value;
}

void QAndroidJniObject::setProperty(const QByteArray &name, jobject value)
{
    QAndroidJniObject o;
    o.d = reinterpret_cast<QAndroidJniObjectPrivate*>(value);
    setProperty(name, QVariant::fromValue(o));
}
