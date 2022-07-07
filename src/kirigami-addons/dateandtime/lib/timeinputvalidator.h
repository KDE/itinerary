/*
 *   SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <QValidator>

class TimeInputValidatorPrivate;

class TimeInputValidator : public QValidator
{
    Q_OBJECT

    /**
     * This property holds the desired time format.
     */
    Q_PROPERTY(QString format READ format WRITE setFormat NOTIFY formatChanged)

public:
    explicit TimeInputValidator(QObject *parent = nullptr);
    ~TimeInputValidator() override;

    // Overrides from QValidator.
    void fixup(QString &input) const override;
    QValidator::State validate(QString &input, int &pos) const override;

    /**
     * Returns the desired time format.
     */
    QString format() const;

    /**
     * Sets the desired time format.
     */
    void setFormat(const QString &format);

Q_SIGNALS:
    void formatChanged();

private:
    QScopedPointer<TimeInputValidatorPrivate> d;

    Q_DISABLE_COPY(TimeInputValidator)
};
