/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

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

#ifndef KPUBLICTRANSPORT_REPLY_H
#define KPUBLICTRANSPORT_REPLY_H

#include <QObject>

#include <memory>

template <typename T> static inline typename std::unique_ptr<T>::pointer qGetPtrHelper(const std::unique_ptr<T> &p) { return p.get(); }

namespace KPublicTransport {

class AbstractBackend;
class Manager;
class ReplyPrivate;

/** Query response base class. */
class Reply : public QObject
{
    Q_OBJECT
public:
    ~Reply();

    /** Error types. */
    enum Error {
        NoError, ///< Nothing went wrong.
        NetworkError, ///< Error during network operations.
        NotFoundError, ///< The requested journey/departure/place could not be found.
        UnknownError ///< Anything else.
    };

    /** Error code. */
    Error error() const;
    /** Textual error message. */
    QString errorString() const;

Q_SIGNALS:
    /** Emitted whenever the journey search has been completed. */
    void finished();

protected:
    explicit Reply(ReplyPrivate *dd);
    std::unique_ptr<ReplyPrivate> d_ptr;

    friend class AbstractBackend;
    void addError(Error error, const QString &errorMsg);

    friend class Manager;
    void setPendingOps(int ops);
};

}

#endif // KPUBLICTRANSPORT_JOURNEYREPLY_H
