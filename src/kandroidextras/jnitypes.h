/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNITYPES_H
#define KANDROIDEXTRAS_JNITYPES_H

namespace KAndroidExtras {

// determine how many elements are in __VA_ARGS__
#define PP_NARG(...) PP_NARG_(__VA_ARGS__, PP_RSEQ_N())
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, N, ...) N
#define PP_RSEQ_N() 7, 6, 5, 4, 3, 2, 1, 0

// preprocessor-level token concat
#define PP_CONCAT(arg1, arg2) PP_CONCAT1(arg1, arg2)
#define PP_CONCAT1(arg1, arg2) PP_CONCAT2(arg1, arg2)
#define PP_CONCAT2(arg1, arg2) arg1##arg2

// preprocessor "iteration"
#define JNI_TYPE_1(name, type, ...) \
    struct type { static constexpr const char* jniName() { return name #type; } };
#define JNI_TYPE_2(name, type, ...) \
    namespace type { JNI_TYPE_1(name #type "/", __VA_ARGS__) }
#define JNI_TYPE_3(name, type, ...) \
    namespace type { JNI_TYPE_2(name #type "/", __VA_ARGS__) }
#define JNI_TYPE_4(name, type, ...) \
    namespace type { JNI_TYPE_3(name #type "/", __VA_ARGS__) }
#define JNI_TYPE_5(name, type, ...) \
    namespace type { JNI_TYPE_4(name #type "/", __VA_ARGS__) }
#define JNI_TYPE_6(name, type, ...) \
    namespace type { JNI_TYPE_5(name #type "/", __VA_ARGS__) }
#define JNI_TYPE_7(name, type, ...) \
    namespace type { JNI_TYPE_6(#type "/", __VA_ARGS__) }
#define JNI_TYPE_(N, name, ...) PP_CONCAT(JNI_TYPE_, N)(name, __VA_ARGS__)

/** Macro to define Java types with their corresponding JNI signature strings. */
#define JNI_TYPE(...) JNI_TYPE_(PP_NARG(__VA_ARGS__), "", __VA_ARGS__)


// type declarations
JNI_TYPE(android, content, ContentResolver)
JNI_TYPE(android, content, Intent)
JNI_TYPE(android, database, Cursor)
JNI_TYPE(android, net, Uri)
JNI_TYPE(android, provider, OpenableColumns)
JNI_TYPE(android, provider, Settings)
JNI_TYPE(java, lang, String)
JNI_TYPE(java, util, Locale)

namespace Jni
{
    /** Returns the JNI type name of the given template argument. */
    template <typename T> inline const char* typeName() { return T::jniName(); }
}

}

#endif // KANDROIDEXTRAS_JNITYPES_H
