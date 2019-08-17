/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KANDROIDEXTRAS_INTENT_H
#define KANDROIDEXTRAS_INTENT_H

#include <QAndroidJniObject>

class QUrl;

namespace KAndroidExtras {

/** Methods to interact with android.content.Intent objects, beyond what QAndroidIntent offers. */
class Intent
{
public:
    /** Creates a new empty intent. */
    Intent();
    /** Adopts an existing intent available as a JNI object. */
    explicit Intent(const QAndroidJniObject &intent);
    ~Intent();

    /** Add a category to the intent. */
    void addCategory(const QAndroidJniObject &category);
    /** Returns the data of this intent. */
    QUrl getData() const;
    /** Sets the action of the intent. */
    void setAction(const QAndroidJniObject &action);
    /** Set the data URL of this intent. */
    void setData(const QUrl &url);
    /** Set the mime type for this intent. */
    void setType(const QString &type);

    /** Implicit conversion to an QAndroidJniObject. */
    operator QAndroidJniObject() const;

    /** Action contstant for create document intents. */
    static QAndroidJniObject ACTION_CREATE_DOCUMENT();
    /** Action contstant for open document intents. */
    static QAndroidJniObject ACTION_OPEN_DOCUMENT();
    /** Action contstant for viewing intents. */
    static QAndroidJniObject ACTION_VIEW();

    /** Category constant for openable content. */
    static QAndroidJniObject CATEGORY_OPENABLE();

private:
    QAndroidJniObject m_intent;
};

}

#endif // KANDROIDEXTRAS_INTENT_H
