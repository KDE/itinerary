/****************************************************************************
**
** SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KFQF-Accepted-GPL OR LicenseRef-Qt-Commercial
**
****************************************************************************/

#ifndef QDATETIMEPARSER_P_H
#define QDATETIMEPARSER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGlobal>
// #include "qplatformdefs.h"
#include <QDateTime>
#include <QStringList>
#include <QLocale>
#include <QVariant>
#include <QVector>
#include <QCoreApplication>

#define QDATETIMEEDIT_TIME_MIN QTime(0, 0, 0, 0)
#define QDATETIMEEDIT_TIME_MAX QTime(23, 59, 59, 999)
#define QDATETIMEEDIT_DATE_MIN QDate(100, 1, 1)
#define QDATETIMEEDIT_COMPAT_DATE_MIN QDate(1752, 9, 14)
#define QDATETIMEEDIT_DATE_MAX QDate(9999, 12, 31)
#define QDATETIMEEDIT_DATETIME_MIN QDateTime(QDATETIMEEDIT_DATE_MIN, QDATETIMEEDIT_TIME_MIN)
#define QDATETIMEEDIT_COMPAT_DATETIME_MIN QDateTime(QDATETIMEEDIT_COMPAT_DATE_MIN, QDATETIMEEDIT_TIME_MIN)
#define QDATETIMEEDIT_DATETIME_MAX QDateTime(QDATETIMEEDIT_DATE_MAX, QDATETIMEEDIT_TIME_MAX)
#define QDATETIMEEDIT_DATE_INITIAL QDate(2000, 1, 1)

class QDateTimeParser
{
    Q_DECLARE_TR_FUNCTIONS(QDateTimeParser)
public:
    enum Context {
        FromString,
        DateTimeEdit
    };
    QDateTimeParser(QVariant::Type t, Context ctx)
        : currentSectionIndex(-1), display(NoSection), cachedDay(-1), parserType(t),
        fixday(false), spec(Qt::LocalTime), context(ctx)
    {
        defaultLocale = QLocale::system();
        first.type = FirstSection;
        first.pos = -1;
        first.count = -1;
        first.zeroesAdded = 0;
        last.type = LastSection;
        last.pos = -1;
        last.count = -1;
        last.zeroesAdded = 0;
        none.type = NoSection;
        none.pos = -1;
        none.count = -1;
        none.zeroesAdded = 0;
    }
    virtual ~QDateTimeParser();

    enum Section {
        NoSection     = 0x00000,
        AmPmSection   = 0x00001,
        MSecSection   = 0x00002,
        SecondSection = 0x00004,
        MinuteSection = 0x00008,
        Hour12Section   = 0x00010,
        Hour24Section   = 0x00020,
        TimeZoneSection = 0x00040,
        HourSectionMask = (Hour12Section | Hour24Section),
        TimeSectionMask = (MSecSection | SecondSection | MinuteSection |
                           HourSectionMask | AmPmSection | TimeZoneSection),

        DaySection         = 0x00100,
        MonthSection       = 0x00200,
        YearSection        = 0x00400,
        YearSection2Digits = 0x00800,
        YearSectionMask = YearSection | YearSection2Digits,
        DayOfWeekSectionShort = 0x01000,
        DayOfWeekSectionLong  = 0x02000,
        DayOfWeekSectionMask = DayOfWeekSectionShort | DayOfWeekSectionLong,
        DaySectionMask = DaySection | DayOfWeekSectionMask,
        DateSectionMask = DaySectionMask | MonthSection | YearSectionMask,

        Internal             = 0x10000,
        FirstSection         = 0x20000 | Internal,
        LastSection          = 0x40000 | Internal,
        CalendarPopupSection = 0x80000 | Internal,

        NoSectionIndex = -1,
        FirstSectionIndex = -2,
        LastSectionIndex = -3,
        CalendarPopupIndex = -4
    }; // extending qdatetimeedit.h's equivalent
    Q_DECLARE_FLAGS(Sections, Section)

    struct Q_CORE_EXPORT SectionNode {
        Section type;
        mutable int pos;
        int count;
        int zeroesAdded;

        static QString name(Section s);
        QString name() const { return name(type); }
        QString format() const;
        int maxChange() const;
    };

    enum State { // duplicated from QValidator
        Invalid,
        Intermediate,
        Acceptable
    };

    struct StateNode {
        StateNode() : state(Invalid), padded(0), conflicts(false) {}
        StateNode(const QDateTime &val, State ok=Acceptable, int pad=0, bool bad=false)
            : value(val), state(ok), padded(pad), conflicts(bad) {}
        QString input;
        QDateTime value;
        State state;
        int padded;
        bool conflicts;
    };

    enum AmPm {
        AmText,
        PmText
    };

    enum Case {
        UpperCase,
        LowerCase
    };

#if QT_CONFIG(datestring)
    StateNode parse(QString input, int position, const QDateTime &defaultValue, bool fixup) const;
    bool fromString(const QString &text, QDate *date, QTime *time) const;
#endif
    bool parseFormat(const QString &format);

    enum FieldInfoFlag {
        Numeric = 0x01,
        FixedWidth = 0x02,
        AllowPartial = 0x04,
        Fraction = 0x08
    };
    Q_DECLARE_FLAGS(FieldInfo, FieldInfoFlag)

    FieldInfo fieldInfo(int index) const;

    void setDefaultLocale(const QLocale &loc) { defaultLocale = loc; }
    virtual QString displayText() const { return text; }

private:
    int sectionMaxSize(Section s, int count) const;
    QString sectionText(const QString &text, int sectionIndex, int index) const;
#if QT_CONFIG(datestring)
    StateNode scanString(const QDateTime &defaultValue,
                         bool fixup, QString *input) const;
    struct ParsedSection {
        int value;
        int used;
        int zeroes;
        State state;
        Q_DECL_CONSTEXPR ParsedSection(State ok = Invalid,
                                       int val = 0, int read = 0, int zs = 0)
            : value(ok == Invalid ? -1 : val), used(read), zeroes(zs), state(ok)
            {}
    };
    ParsedSection parseSection(const QDateTime &currentValue, int sectionIndex,
                               int offset, QString *text) const;
    int findMonth(const QString &str1, int monthstart, int sectionIndex,
                  QString *monthName = nullptr, int *used = nullptr) const;
    int findDay(const QString &str1, int intDaystart, int sectionIndex,
                QString *dayName = nullptr, int *used = nullptr) const;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    ParsedSection findTimeZone(QStringRef str, const QDateTime &when,
                               int maxVal, int minVal) const;
#else
    ParsedSection findTimeZone(QStringView str, const QDateTime &when,
                               int maxVal, int minVal) const;
#endif
#if QT_CONFIG(timezone)
    // Implemented in qdatetime.cpp:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    static int startsWithLocalTimeZone(const QStringRef name);
#else
    static int startsWithLocalTimeZone(const QStringView name);
#endif
#endif

    enum AmPmFinder {
        Neither = -1,
        AM = 0,
        PM = 1,
        PossibleAM = 2,
        PossiblePM = 3,
        PossibleBoth = 4
    };
    AmPmFinder findAmPm(QString &str, int index, int *used = nullptr) const;
#endif // datestring
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    bool potentialValue(const QStringRef &str, int min, int max, int index,
                        const QDateTime &currentValue, int insert) const;
    bool potentialValue(const QString &str, int min, int max, int index,
                        const QDateTime &currentValue, int insert) const
    {
        return potentialValue(QStringRef(&str), min, max, index, currentValue, insert);
    }
#else
    bool potentialValue(const QStringView &str, int min, int max, int index,
                        const QDateTime &currentValue, int insert) const;
    bool potentialValue(const QString &str, int min, int max, int index,
                        const QDateTime &currentValue, int insert) const
    {
        return potentialValue(QStringView(str), min, max, index, currentValue, insert);
    }

#endif

protected: // for the benefit of QDateTimeEditPrivate
    int sectionSize(int index) const;
    int sectionMaxSize(int index) const;
    int sectionPos(int index) const;
    int sectionPos(const SectionNode &sn) const;

    const SectionNode &sectionNode(int index) const;
    Section sectionType(int index) const;
    QString sectionText(int sectionIndex) const;
    int getDigit(const QDateTime &dt, int index) const;
    bool setDigit(QDateTime &t, int index, int newval) const;

    int absoluteMax(int index, const QDateTime &value = QDateTime()) const;
    int absoluteMin(int index) const;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    bool skipToNextSection(int section, const QDateTime &current, const QStringRef &sectionText) const;
    bool skipToNextSection(int section, const QDateTime &current, const QString &sectionText) const
    {
        return skipToNextSection(section, current, QStringRef(&sectionText));
    }
#else
    bool skipToNextSection(int section, const QDateTime &current, const QStringView &sectionText) const;
    bool skipToNextSection(int section, const QDateTime &current, const QString &sectionText) const
    {
        return skipToNextSection(section, current, QStringView(sectionText));
    }

#endif
    QString stateName(State s) const;
    virtual QDateTime getMinimum() const;
    virtual QDateTime getMaximum() const;
    virtual int cursorPosition() const { return -1; }
    virtual QString getAmPmText(AmPm ap, Case cs) const;
    virtual QLocale locale() const { return defaultLocale; }

    mutable int currentSectionIndex;
    Sections display;
    /*
        This stores the most recently selected day.
        It is useful when considering the following scenario:

        1. Date is: 31/01/2000
        2. User increments month: 29/02/2000
        3. User increments month: 31/03/2000

        At step 1, cachedDay stores 31. At step 2, the 31 is invalid for February, so the cachedDay is not updated.
        At step 3, the month is changed to March, for which 31 is a valid day. Since 29 < 31, the day is set to cachedDay.
        This is good for when users have selected their desired day and are scrolling up or down in the month or year section
        and do not want smaller months (or non-leap years) to alter the day that they chose.
    */
    mutable int cachedDay;
    mutable QString text;
    QVector<SectionNode> sectionNodes;
    SectionNode first, last, none, popup;
    QStringList separators;
    QString displayFormat;
    QLocale defaultLocale;
    QVariant::Type parserType;
    bool fixday;
    Qt::TimeSpec spec; // spec if used by QDateTimeEdit
    Context context;
};
Q_DECLARE_TYPEINFO(QDateTimeParser::SectionNode, Q_PRIMITIVE_TYPE);

Q_CORE_EXPORT bool operator==(const QDateTimeParser::SectionNode &s1, const QDateTimeParser::SectionNode &s2);

Q_DECLARE_OPERATORS_FOR_FLAGS(QDateTimeParser::Sections)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDateTimeParser::FieldInfo)

QT_END_NAMESPACE

#endif // QDATETIME_P_H
