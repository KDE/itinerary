/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef FAKE_JNI_H
#define FAKE_JNI_H

#include <cstdint>

#ifdef Q_OS_ANDROID
#error This is a mock object for use on non-Android!
#endif

typedef uint8_t jboolean;
typedef int8_t jbyte;
typedef uint16_t jchar;
typedef int16_t jshort;
typedef int32_t jint;
typedef int64_t jlong;
typedef float jfloat;
typedef double jdouble;

typedef void* jobject;
typedef jobject jclass;
typedef jobject jstring;
typedef jobject jarray;
typedef jarray jobjectArray;

struct JNIEnv
{
    inline bool ExceptionCheck() { return false; }
    inline void ExceptionClear() {}
    inline int GetArrayLength(jobjectArray) { return m_arrayLength; }
    inline jobject GetObjectArrayElement(jobjectArray, int index) { return reinterpret_cast<jobject>(index); }

    static int m_arrayLength;
};

#endif
