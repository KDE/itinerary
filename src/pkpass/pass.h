/*
   Copyright (c) 2017-2018 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#ifndef KPKPASS_PASS_H
#define KPKPASS_PASS_H

#include "kpkpass_export.h"
#include "barcode.h"
#include "field.h"

#include <QByteArray>
#include <QColor>
#include <QDateTime>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QVariant>
#include <QVector>

#include <memory>

class KZip;
class QIODevice;
class QJsonObject;
class QLatin1String;
class QString;

namespace KPkPass {

class Barcode;

/** Base class for a pkpass file.
 *  @see https://developer.apple.com/library/content/documentation/UserExperience/Reference/PassKit_Bundle/Chapters/TopLevel.html
 */
class KPKPASS_EXPORT Pass : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString passTypeIdentifier READ passTypeIdentifier CONSTANT)
    Q_PROPERTY(QString serialNumber READ serialNumber CONSTANT)
    Q_PROPERTY(Type type READ type CONSTANT)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor CONSTANT)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor CONSTANT)
    Q_PROPERTY(QColor labelColor READ labelColor CONSTANT)
    Q_PROPERTY(QString logoText READ logoText CONSTANT)
    Q_PROPERTY(QDateTime relevantDate READ relevantDate CONSTANT)

    // needs to be QVariantList just for QML (Grantlee would also work with QVector<Field>
    Q_PROPERTY(QVariantList auxiliaryFields READ auxiliaryFieldsVariant CONSTANT)
    Q_PROPERTY(QVariantList backFields READ backFieldsVariant CONSTANT)
    Q_PROPERTY(QVariantList headerFields READ headerFieldsVariant CONSTANT)
    Q_PROPERTY(QVariantList primaryFields READ primaryFieldsVariant CONSTANT)
    Q_PROPERTY(QVariantList secondaryFields READ secondaryFieldsVariant CONSTANT)

    Q_PROPERTY(QVariantList barcodes READ barcodesVariant CONSTANT)

public:
    virtual ~Pass();

    /** Type of the pass. */
    enum Type {
        BoardingPass,
        Coupon,
        EventTicket,
        Generic,
        StoreCard
    };
    Q_ENUM(Type)
    Type type() const;

    /** Content of the pass.json file. */
    QJsonObject data() const;
    /** The pass data structure of the pass.json file. */
    QJsonObject passData() const;
    /** Localized message for the given key. */
    QString message(const QString &key) const;

    QString passTypeIdentifier() const;
    QString serialNumber() const;

    QColor backgroundColor() const;
    QColor foregroundColor() const;
    QColor labelColor() const;
    QString logoText() const;
    QImage logo(unsigned int devicePixelRatio = 1) const;
    QDateTime relevantDate() const;

    QVector<Field> auxiliaryFields() const;
    QVector<Field> backFields() const;
    QVector<Field> headerFields() const;
    QVector<Field> primaryFields() const;
    QVector<Field> secondaryFields() const;

    /** Returns all barcodes defined in the pass. */
    QVector<Barcode> barcodes() const;

    /** Create a appropriate sub-class based on the pkpass file type. */
    static Pass *fromData(const QByteArray &data, QObject *parent = nullptr);
    /** Create a appropriate sub-class based on the pkpass file type. */
    static Pass *fromFile(const QString &fileName, QObject *parent = nullptr);

protected:
    ///@cond internal
    explicit Pass (const QString &passType, QObject *parent = nullptr);
    ///@endcond

private:
    static Pass *fromData(std::unique_ptr<QIODevice> device, QObject *parent);
    void parse();
    bool parseMessages(const QString &lang);

    QVector<Field> fields(const QLatin1String &fieldType) const;
    QVariantList auxiliaryFieldsVariant() const;
    QVariantList backFieldsVariant() const;
    QVariantList headerFieldsVariant() const;
    QVariantList primaryFieldsVariant() const;
    QVariantList secondaryFieldsVariant() const;
    QVariantList barcodesVariant() const;

    std::unique_ptr<QIODevice> m_buffer;
    std::unique_ptr<KZip> m_zip;
    QJsonObject m_passObj;
    QHash<QString, QString> m_messages;
    QString m_passType;
};

}

#endif // KPKPASS_PASS_H
