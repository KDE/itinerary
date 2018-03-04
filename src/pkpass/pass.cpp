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

#include "pass.h"
#include "boardingpass.h"
#include "logging.h"

#include <KZip>

#include <QBuffer>
#include <QColor>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QTextCodec>
#include <QImage>

using namespace KPkPass;

Pass::Pass (const QString &passType, QObject *parent)
    : QObject(parent)
    , m_passType(passType)
{
}

Pass::~Pass() = default;

QJsonObject Pass::data() const
{
    return m_passObj;
}

QJsonObject Pass::passData() const
{
    return m_passObj.value(m_passType).toObject();
}

QString Pass::message(const QString &key) const
{
    const auto it = m_messages.constFind(key);
    if (it != m_messages.constEnd()) {
        return it.value();
    }
    return key;
}

QString Pass::passTypeIdentifier() const
{
    return m_passObj.value(QLatin1String("passTypeIdentifier")).toString();
}

QString Pass::serialNumber() const
{
    return m_passObj.value(QLatin1String("serialNumber")).toString();
}

static const char* passTypes[] = { "boardingPass", "coupon", "eventTicket", "generic", "storeCard" };
static const auto passTypesCount = sizeof(passTypes) / sizeof(passTypes[0]);

Pass::Type KPkPass::Pass::type() const
{
    for (unsigned int i = 0; i < passTypesCount; ++i) {
        if (data().contains(QLatin1String(passTypes[i]))) {
            return static_cast<Type>(i);
        }
    }
    qCWarning(Log) << "pkpass file has no pass data structure!";
    return Generic;
}

static QColor parseColor(const QString &s)
{
    if (s.startsWith(QLatin1String("rgb("), Qt::CaseInsensitive)) {
        const auto l = s.midRef(4, s.length() - 5).split(QLatin1Char(','));
        if (l.size() != 3)
            return {};
        return QColor(l[0].trimmed().toInt(), l[1].trimmed().toInt(), l[2].trimmed().toInt());
    }
    return QColor(s);
}

QColor Pass::backgroundColor() const
{
    return parseColor(m_passObj.value(QLatin1String("backgroundColor")).toString());
}

QColor Pass::foregroundColor() const
{
    return parseColor(m_passObj.value(QLatin1String("foregroundColor")).toString());
}

QColor Pass::labelColor() const
{
    const auto c = parseColor(m_passObj.value(QLatin1String("labelColor")).toString());
    if (c.isValid()) {
        return c;
    }
    return foregroundColor();
}

QString Pass::logoText() const
{
    return message(m_passObj.value(QLatin1String("logoText")).toString());
}

QImage Pass::logo(unsigned int devicePixelRatio) const
{
    const KArchiveFile *file = nullptr;
    for (; devicePixelRatio > 1; --devicePixelRatio) {
        file = m_zip->directory()->file(QLatin1String("logo@") + QString::number(devicePixelRatio) + QLatin1String("x.png"));
        if (file)
            break;
    }
    if (!file)
        file = m_zip->directory()->file(QLatin1String("logo.png"));
    if (!file)
        return {};
    std::unique_ptr<QIODevice> dev(file->createDevice());
    auto img = QImage::fromData(dev->readAll());
    img.setDevicePixelRatio(devicePixelRatio);
    return img;
}

QDateTime Pass::relevantDate() const
{
    return QDateTime::fromString(m_passObj.value(QLatin1String("relevantDate")).toString(), Qt::ISODate);
}

QVector<Barcode> Pass::barcodes() const
{
    QVector<Barcode> codes;

    // barcodes array
    const auto a = data().value(QLatin1String("barcodes")).toArray();
    codes.reserve(a.size());
    for (const auto &bc : a)
        codes.push_back(Barcode(bc.toObject(), this));

    // just a single barcode
    if (codes.isEmpty()) {
        const auto bc = data().value(QLatin1String("barcode")).toObject();
        if (!bc.isEmpty())
            codes.push_back(Barcode(bc, this));
    }

    return codes;
}

QVector<Field> Pass::auxiliaryFields() const
{
    return fields(QLatin1String("auxiliaryFields"));
}

QVector<Field> Pass::backFields() const
{
    return fields(QLatin1String("backFields"));
}

QVector<Field> Pass::headerFields() const
{
    return fields(QLatin1String("headerFields"));
}

QVector<Field> Pass::primaryFields() const
{
    return fields(QLatin1String("primaryFields"));
}

QVector<Field> Pass::secondaryFields() const
{
    return fields(QLatin1String("secondaryFields"));
}

Pass *Pass::fromData(const QByteArray &data, QObject *parent)
{
    std::unique_ptr<QBuffer> buffer(new QBuffer);
    buffer->setData(data);
    buffer->open(QBuffer::ReadOnly);
    return fromData(std::move(buffer), parent);
}

Pass *Pass::fromData(std::unique_ptr<QIODevice> device, QObject *parent)
{
    std::unique_ptr<KZip> zip(new KZip(device.get()));
    if (!zip->open(QIODevice::ReadOnly)) {
        return nullptr;
    }

    // extract pass.json
    auto file = zip->directory()->file(QStringLiteral("pass.json"));
    if (!file) {
        return nullptr;
    }
    std::unique_ptr<QIODevice> dev(file->createDevice());
    const auto passObj = QJsonDocument::fromJson(dev->readAll()).object();

    Pass *pass = nullptr;
    if (passObj.contains(QLatin1String("boardingPass"))) {
        pass = new KPkPass::BoardingPass(parent);
    }
    // TODO: coupon, eventTicket, storeCard, generic
    else {
        pass = new Pass (QStringLiteral("generic"), parent);
    }

    pass->m_buffer = std::move(device);
    pass->m_zip = std::move(zip);
    pass->m_passObj = passObj;
    pass->parse();
    return pass;
}

Pass *Pass::fromFile(const QString &fileName, QObject *parent)
{
    std::unique_ptr<QFile> file(new QFile(fileName));
    if (file->open(QFile::ReadOnly)) {
        return fromData(std::move(file), parent);
    }
    qCWarning(Log) << "Failed to open" << fileName << ":" << file->errorString();
    return nullptr;
}

void Pass::parse()
{
    // find the message catalog
    auto lang = QLocale().name();
    auto idx = lang.indexOf(QLatin1Char('_'));
    if (idx > 0) {
        lang = lang.left(idx);
    }
    lang += QLatin1String(".lproj");
    if (!parseMessages(lang)) {
        parseMessages(QStringLiteral("en.lproj"));
    }
}

static int indexOfUnquoted(const QString &catalog, QLatin1Char c, int start)
{
    for (int i = start; i < catalog.size(); ++i) {
        const QChar catalogChar = catalog.at(i);
        if (catalogChar == c) {
            return i;
        }
        if (catalogChar == QLatin1Char('\\')) {
            ++i;
        }
    }

    return -1;
}

static QString unquote(const QStringRef &str)
{
    QString res;
    res.reserve(str.size());
    for (int i = 0; i < str.size(); ++i) {
        const auto c1 = str.at(i);
        if (c1 == QLatin1Char('\\') && i < str.size() - 1) {
            const auto c2 = str.at(i + 1);
            if (c2 == QLatin1Char('r')) {
                res.push_back(QLatin1Char('\r'));
            } else if (c2 == QLatin1Char('n')) {
                res.push_back(QLatin1Char('\n'));
            } else if (c2 == QLatin1Char('\\')) {
                res.push_back(c2);
            } else {
                res.push_back(c1);
                res.push_back(c2);
            }
            ++i;
        } else {
            res.push_back(c1);
        }
    }
    return res;
}

bool Pass::parseMessages(const QString &lang)
{
    auto entry = m_zip->directory()->entry(lang);
    if (!entry || !entry->isDirectory()) {
        return false;
    }

    auto dir = static_cast<const KArchiveDirectory *>(entry);
    auto file = dir->file(QStringLiteral("pass.strings"));
    if (!file) {
        return false;
    }

    std::unique_ptr<QIODevice> dev(file->createDevice());
    const auto rawData = dev->readAll();
    // this should be UTF-16BE, but that doesn't stop Eurowings from using UTF-8,
    // so do a primitive auto-detection here. UTF-16's first byte would either be the BOM
    // or \0.
    QString catalog;
    if (rawData.at(0) == '"') {
        catalog = QString::fromUtf8(rawData);
    } else {
        auto codec = QTextCodec::codecForName("UTF-16BE");
        catalog = codec->toUnicode(rawData);
    }

    int idx = 0;
    while (idx < catalog.size()) {
        // key
        const auto keyBegin = indexOfUnquoted(catalog, QLatin1Char('"'), idx) + 1;
        if (keyBegin < 1) {
            break;
        }
        const auto keyEnd = indexOfUnquoted(catalog, QLatin1Char('"'), keyBegin);
        if (keyEnd <= keyBegin) {
            break;
        }

        // value
        const auto valueBegin = indexOfUnquoted(catalog, QLatin1Char('"'), keyEnd + 2) + 1; // there's at least also the '='
        if (valueBegin <= keyEnd) {
            break;
        }
        const auto valueEnd = indexOfUnquoted(catalog, QLatin1Char('"'), valueBegin);
        if (valueEnd <= valueBegin) {
            break;
        }

        const auto key = catalog.mid(keyBegin, keyEnd - keyBegin);
        const auto value = unquote(catalog.midRef(valueBegin, valueEnd - valueBegin));
        m_messages.insert(key, value);
        idx = valueEnd + 1; // there's at least the linebreak and/or a ';'
    }

    return !m_messages.isEmpty();
}

QVector<Field> Pass::fields(const QLatin1String &fieldType) const
{
    const auto a = passData().value(fieldType).toArray();
    QVector<Field> f;
    f.reserve(a.size());
    foreach (const auto &v, a) {
        f.push_back(Field{v.toObject(), this});
    }
    return f;
}

template <typename T>
static QVariantList toVariantList(const QVector<T> &elems)
{
    QVariantList l;
    l.reserve(elems.size());
    std::for_each(elems.begin(), elems.end(), [&l](const T &e) { l.push_back(QVariant::fromValue(e)); });
    return l;
}

QVariantList Pass::auxiliaryFieldsVariant() const
{
    return toVariantList(auxiliaryFields());
}

QVariantList Pass::backFieldsVariant() const
{
    return toVariantList(backFields());
}

QVariantList Pass::headerFieldsVariant() const
{
    return toVariantList(headerFields());
}

QVariantList Pass::primaryFieldsVariant() const
{
    return toVariantList(primaryFields());
}

QVariantList Pass::secondaryFieldsVariant() const
{
    return toVariantList(secondaryFields());
}

QVariantList Pass::barcodesVariant() const
{
    return toVariantList(barcodes());
}
