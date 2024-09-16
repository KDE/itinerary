/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "intenthandler.h"
#include "logging.h"

#ifdef Q_OS_ANDROID
#include "android/itineraryactivity.h"

#include "kandroidextras/intent.h"
#endif

class IntentHandlerPrivate {
public:
    static void onNewIntent(const KAndroidExtras::Intent &intent);
};

void IntentHandlerPrivate::onNewIntent(const KAndroidExtras::Intent &intent)
{
    if (IntentHandler::s_instance) {
        IntentHandler::s_instance->onNewIntent(intent);
    }
}

IntentHandler* IntentHandler::s_instance = nullptr;

#ifdef Q_OS_ANDROID

// TODO rename as this is now more generic than just importing
static void importFromIntent(JNIEnv *env, jobject that, jobject data)
{
    Q_UNUSED(that)
    Q_UNUSED(env)
    IntentHandlerPrivate::onNewIntent(KAndroidExtras::Jni::fromHandle<KAndroidExtras::Intent>(data));
}

static const JNINativeMethod methods[] = {
    {"importFromIntent", "(Landroid/content/Intent;)V", (void*)importFromIntent},
};

Q_DECL_EXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void*)
{
    static bool initialized = false;
    if (initialized)
        return JNI_VERSION_1_6;
    initialized = true;

    JNIEnv *env = nullptr;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
        qCWarning(Log) << "Failed to get JNI environment.";
        return -1;
    }
    jclass cls = env->FindClass(KAndroidExtras::Jni::typeName<ItineraryActivity>());
    if (env->RegisterNatives(cls, methods, sizeof(methods) / sizeof(JNINativeMethod)) < 0) {
        qCWarning(Log) << "Failed to register native functions.";
        return -1;
    }

    return JNI_VERSION_1_4;
}
#endif

IntentHandler::IntentHandler(QObject *parent)
    : QObject(parent)
{
    Q_ASSERT(!s_instance);
    s_instance = this;
}

IntentHandler::~IntentHandler()
{
    s_instance = nullptr;
}

void IntentHandler::onNewIntent(const KAndroidExtras::Intent &intent)
{
#ifdef Q_OS_ANDROID
    // For yet unknown reasons the normal signal emission by calling the
    // function directly fails here / the thread that recerives the intent.
    // QMetaObject::invokeMethod works.
    QMetaObject::invokeMethod(this, &IntentHandler::handleIntent, intent);
#endif
}
