// SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef QMLREGISTRATIONHELPER_H
#define QMLREGISTRATIONHELPER_H

#include <QQmlEngine>

#define REGISTER_SINGLETON_INSTANCE(Class) \
struct Class##Instance \
{ \
    Q_GADGET \
    QML_FOREIGN(Class) \
    QML_SINGLETON \
    QML_NAMED_ELEMENT(Class) \
public: \
    inline static Class *instance = nullptr; \
    static Class *create(QQmlEngine*, QJSEngine*) \
    { \
        Q_ASSERT(instance); \
        QJSEngine::setObjectOwnership(instance, QJSEngine::CppOwnership); \
        return instance; \
    } \
};

#endif
