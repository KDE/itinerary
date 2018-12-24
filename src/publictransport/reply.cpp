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

#include "reply.h"
#include "reply_p.h"

using namespace KPublicTransport;

void ReplyPrivate::emitFinishedIfDone(Reply *q)
{
    if (pendingOps <= 0) {
        finalizeResult();
        emit q->finished();
    }
}

Reply::Reply(ReplyPrivate *dd)
    : d_ptr(dd)
{
}

Reply::~Reply() = default;

Reply::Error Reply::error() const
{
    return d_ptr->error;
}

QString Reply::errorString() const
{
    return d_ptr->errorMsg;
}

void Reply::addError(Reply::Error error, const QString &errorMsg)
{
    d_ptr->error = error;
    d_ptr->errorMsg = errorMsg;
    d_ptr->pendingOps--;
    d_ptr->emitFinishedIfDone(this);
}

void Reply::setPendingOps(int ops)
{
    Q_ASSERT(d_ptr->pendingOps <= -1);
    Q_ASSERT(ops >= 0);
    d_ptr->pendingOps = ops;
    if (ops == 0) {
        QMetaObject::invokeMethod(this, &Reply::finished, Qt::QueuedConnection);
    }
}
