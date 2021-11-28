/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LOCATIONINFORMATIONDELEGATECONTROLLER_H
#define LOCATIONINFORMATIONDELEGATECONTROLLER_H

#include "locationinformation.h"

#include <QObject>

/** Logic for location information delegates. */
class LocationInformationDelegateController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(LocationInformation locationInformation READ locationInformation WRITE setLocationInformation NOTIFY infoChanged)
    Q_PROPERTY(QString homeCurrencyCode READ homeCurrencyCode WRITE setHomeCurrencyCode NOTIFY homeCurrencyCodeChanged)
    Q_PROPERTY(bool performCurrencyConversion READ performCurrencyConversion WRITE setPerformCurrencyConversion NOTIFY performCurrencyConversionChanged)
    Q_PROPERTY(bool hasCurrencyConversion READ hasCurrencyConversion NOTIFY currencyConversionChanged)
    Q_PROPERTY(QString currencyConversionLabel READ currencyConversionLabel NOTIFY currencyConversionChanged)

public:
    explicit LocationInformationDelegateController(QObject *parent = nullptr);
    ~LocationInformationDelegateController();

    LocationInformation locationInformation() const;
    void setLocationInformation(const LocationInformation &info);

    QString homeCurrencyCode() const;
    void setHomeCurrencyCode(const QString &currencyCode);

    bool performCurrencyConversion() const;
    void setPerformCurrencyConversion(bool enable);

    bool hasCurrencyConversion() const;
    QString currencyConversionLabel() const;

Q_SIGNALS:
    void infoChanged();
    void homeCurrencyCodeChanged();
    void currencyConversionChanged();
    void performCurrencyConversionChanged();

private:
    void recheckCurrencyConversion();

    LocationInformation m_info;
    QString m_homeCurrency;
    float m_conversionRate = 0.0f;
    bool m_performCurrencyConverion = false;
};

#endif // LOCATIONINFORMATIONDELEGATECONTROLLER_H
