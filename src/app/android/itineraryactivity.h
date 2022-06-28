/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ITINERARY_ACTIVITY_H
#define ITINERARY_ACTIVITY_H

#include <kandroidextras/jnitypes.h>
#include <kandroidextras/jnimethod.h>
#include <kandroidextras/jniproperty.h>
#include <kandroidextras/javatypes.h>
#include <kandroidextras/androidtypes.h>
#include <kandroidextras/uri.h>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtAndroid>
#else
#include <QCoreApplication>
#endif

JNI_TYPE(org, kde, itinerary, Activity)

/** Interface to the Java Activity class. */
class ItineraryActivity {
    JNI_UNMANAGED_OBJECT(ItineraryActivity, org::kde::itinerary::Activity)
public:
    JNI_METHOD(KAndroidExtras::Jni::Array<KAndroidExtras::java::lang::String>, attachmentsForIntent, KAndroidExtras::android::content::Intent)
    JNI_METHOD(KAndroidExtras::android::net::Uri, openDocument, KAndroidExtras::java::lang::String)
private:
    inline QAndroidJniObject jniHandle() const {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        return QtAndroid::androidActivity();
#else
        return QNativeInterface::QAndroidApplication::context();
#endif
    }
};

#endif
