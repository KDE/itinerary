/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_INTENT_H
#define KANDROIDEXTRAS_INTENT_H

#include "kandroidextras_export.h"

#include <KAndroidExtras/AndroidTypes>
#include <KAndroidExtras/JniProperty>

#include <QAndroidJniObject>

class QUrl;

namespace KAndroidExtras {

/** Methods to interact with android.content.Intent objects.
 *  This does not only offer features beyond what QAndroidIntent, it also provides
 *  a putExtra() implementation that actually interoperates with system services.
 */
class KANDROIDEXTRAS_EXPORT Intent : Jni::Wrapper<android::content::Intent>
{
public:
    /** Creates a new empty intent. */
    Intent();
    /** Adopts an existing intent available as a JNI object. */
    explicit Intent(const QAndroidJniObject &intent);
    ~Intent();

    /** Add a category to the intent. */
    void addCategory(const QAndroidJniObject &category);
    /** Add flags to this intent. */
    void addFlags(jint flags);
    /** Returns the data of this intent. */
    QUrl getData() const;
    /** Get the intent action. */
    QString getAction() const;
    /** Sets the action of the intent. */
    void setAction(const QAndroidJniObject &action);
    /** Set the data URL of this intent. */
    void setData(const QUrl &url);
    void setData(const QAndroidJniObject &uri);

    /** Returns the mimetype of this intent. */
    QString getType() const;
    /** Set the mime type for this intent. */
    void setType(const QString &type);

    /** Read extra intent data. */
    QString getStringExtra(const QAndroidJniObject &name) const;
    QStringList getStringArrayExtra(const QAndroidJniObject &name) const;
    /** Add extra intent data of type @tparam T. */
    template <typename T>
    inline void putExtra(const QAndroidJniObject &name, const QAndroidJniObject &value)
    {
        m_intent.callObjectMethod("putExtra", Jni::signature<android::content::Intent(java::lang::String, T)>(), name.object(), value.object());
    }

    /** Implicit conversion to an QAndroidJniObject. */
    operator QAndroidJniObject() const;

    /** Action constant for create document intents. */
    JNI_CONSTANT(java::lang::String, ACTION_CREATE_DOCUMENT)
    /** Main activity entry point. */
    JNI_CONSTANT(java::lang::String, ACTION_MAIN)
    /** Action constant for open document intents. */
    JNI_CONSTANT(java::lang::String, ACTION_OPEN_DOCUMENT)
    /** Action constant for viewing intents. */
    JNI_CONSTANT(java::lang::String, ACTION_VIEW)
    /** Share data. */
    JNI_CONSTANT(java::lang::String, ACTION_SEND)
    /** Share multiple data items. */
    JNI_CONSTANT(java::lang::String, ACTION_SEND_MULTIPLE)

    /** Category constant for openable content. */
    JNI_CONSTANT(java::lang::String, CATEGORY_OPENABLE)

    JNI_CONSTANT(java::lang::String, EXTRA_EMAIL)
    JNI_CONSTANT(java::lang::String, EXTRA_STREAM)
    JNI_CONSTANT(java::lang::String, EXTRA_SUBJECT)
    JNI_CONSTANT(java::lang::String, EXTRA_TEXT)

    /** Flag for granting read URI permissions on content providers. */
    JNI_CONSTANT(jint, FLAG_GRANT_READ_URI_PERMISSION)
    /** Flag for granting write URI permissions on content providers. */
    JNI_CONSTANT(jint, FLAG_GRANT_WRITE_URI_PERMISSION)

private:
    template <typename T>
    QAndroidJniObject getObjectExtra(const char* methodName, const QAndroidJniObject &name) const;
    QAndroidJniObject m_intent;
};

}

#endif // KANDROIDEXTRAS_INTENT_H
