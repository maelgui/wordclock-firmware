/**************************************************************************/
/*!
  @file     RTClib.h

  Original library by JeeLabs http://news.jeelabs.org/code/, released to the public domain

  License: MIT (see LICENSE)

  This is a fork of JeeLab's fantastic real time clock library for Arduino.

  For details on using this library with an RTC module like the DS1307, PCF8523, or DS3231,
  see the guide at: https://learn.adafruit.com/ds1307-real-time-clock-breakout-board-kit/overview

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
*/
/**************************************************************************/

#ifndef _RTCLIB_H_
#define _RTCLIB_H_

#include <avr/io.h>

class TimeSpan;

/** Registers */
#define DS3231_ADDRESS        0x68  ///< I2C address for DS3231
#define DS3231_CONTROL        0x0E  ///< Control register
#define DS3231_STATUSREG      0x0F  ///< Status register
#define DS3231_TEMPERATUREREG	0x11  ///< Temperature register (high byte - low byte is at 0x12), 10-bit temperature value

/** Constants */
#define SECONDS_PER_DAY       86400L  ///< 60 * 60 * 24
#define SECONDS_FROM_1970_TO_2000 946684800  ///< Unixtime for 2000-01-01 00:00:00, useful for initialization


/**************************************************************************/
/*!
    @brief  Simple general-purpose date/time class (no TZ / DST / leap second handling!).
            See http://en.wikipedia.org/wiki/Leap_second
*/
/**************************************************************************/
class DateTime {
public:
    DateTime (uint32_t t = SECONDS_FROM_1970_TO_2000);
    DateTime (uint16_t year, uint8_t month, uint8_t day,
              uint8_t hour = 0, uint8_t min = 0, uint8_t sec = 0);
    DateTime (const DateTime& copy);
    DateTime (const char* date, const char* time);
    char* toString(char* buffer);

    /*!
        @brief  Return the year, stored as an offset from 2000
        @return uint16_t year
    */
    uint16_t year() const       { return 2000 + yOff; }
    /*!
        @brief  Return the year, stored as an offset from 2000
        @return uint16_t year
    */
    uint8_t rawYear() const       { return yOff; }
    /*!
        @brief  Return month
        @return uint8_t month
    */
    uint8_t month() const       { return m; }
    /*!
        @brief  Return day
        @return uint8_t day
    */
    uint8_t day() const         { return d; }
    /*!
        @brief  Return hours
        @return uint8_t hours
    */
    uint8_t hour() const        { return hh; }
    /*!
        @brief  Return minutes
        @return uint8_t minutes
    */
    uint8_t minute() const      { return mm; }
    /*!
        @brief  Return seconds
        @return uint8_t seconds
    */
    uint8_t second() const      { return ss; }

    uint8_t dayOfTheWeek() const;

    /** 32-bit times as seconds since 1/1/2000 */
    long secondstime() const;

    /** 32-bit times as seconds since 1/1/1970 */
    uint32_t unixtime(void) const;

    /** ISO 8601 Timestamp function */
    enum timestampOpt{
        TIMESTAMP_FULL, // YYYY-MM-DDTHH:MM:SS
        TIMESTAMP_TIME, // HH:MM:SS
        TIMESTAMP_DATE  // YYYY-MM-DD
    };
    //char* timestamp(timestampOpt opt = TIMESTAMP_FULL);

    DateTime operator+(const TimeSpan& span);
    DateTime operator-(const TimeSpan& span);
    TimeSpan operator-(const DateTime& right);
    bool operator<(const DateTime& right) const;
    /*!
        @brief  Test if one DateTime is greater (later) than another
        @param right DateTime object to compare
        @return True if the left object is greater than the right object, false otherwise
    */
    bool operator>(const DateTime& right) const  { return right < *this; }
    /*!
        @brief  Test if one DateTime is less (earlier) than or equal to another
        @param right DateTime object to compare
        @return True if the left object is less than or equal to the right object, false otherwise
    */
    bool operator<=(const DateTime& right) const { return !(*this > right); }
    /*!
        @brief  Test if one DateTime is greater (later) than or equal to another
        @param right DateTime object to compare
        @return True if the left object is greater than or equal to the right object, false otherwise
    */
    bool operator>=(const DateTime& right) const { return !(*this < right); }
    bool operator==(const DateTime& right) const;
    /*!
        @brief  Test if two DateTime objects not equal
        @param right DateTime object to compare
        @return True if the two objects are not equal, false if they are
    */
    bool operator!=(const DateTime& right) const { return !(*this == right); }

    DateTime& operator=(const DateTime& copy);

protected:
    uint8_t yOff;   ///< Year offset from 2000
    uint8_t m;      ///< Month 1-12
    uint8_t d;      ///< Day 1-31
    uint8_t hh;     ///< Hours 0-23
    uint8_t mm;     ///< Minutes 0-59
    uint8_t ss;     ///< Seconds 0-59
};


/**************************************************************************/
/*!
    @brief  Timespan which can represent changes in time with seconds accuracy.
*/
/**************************************************************************/
class TimeSpan {
public:
    TimeSpan (int32_t seconds = 0);
    TimeSpan (int16_t days, int8_t hours, int8_t minutes, int8_t seconds);
    TimeSpan (const TimeSpan& copy);

    /*!
        @brief  Number of days in the TimeSpan
                e.g. 4
        @return int16_t days
    */
    int16_t days() const         { return _seconds / 86400L; }
    /*!
        @brief  Number of hours in the TimeSpan
                This is not the total hours, it includes the days
                e.g. 4 days, 3 hours - NOT 99 hours
        @return int8_t hours
    */
    int8_t  hours() const        { return _seconds / 3600 % 24; }
    /*!
        @brief  Number of minutes in the TimeSpan
                This is not the total minutes, it includes days/hours
                e.g. 4 days, 3 hours, 27 minutes
        @return int8_t minutes
    */
    int8_t  minutes() const      { return _seconds / 60 % 60; }
    /*!
        @brief  Number of seconds in the TimeSpan
                This is not the total seconds, it includes the days/hours/minutes
                e.g. 4 days, 3 hours, 27 minutes, 7 seconds
        @return int8_t seconds
    */
    int8_t  seconds() const      { return _seconds % 60; }
    /*!
        @brief  Total number of seconds in the TimeSpan, e.g. 358027
        @return int32_t seconds
    */
    int32_t totalseconds() const { return _seconds; }

    TimeSpan operator+(const TimeSpan& right);
    TimeSpan operator-(const TimeSpan& right);

protected:
    int32_t _seconds;   ///< Actual TimeSpan value is stored as seconds
};



/** DS3231 SQW pin mode settings */
enum Ds3231SqwPinMode {
    DS3231_OFF            = 0x01, // Off
    DS3231_SquareWave1Hz  = 0x00, // 1Hz square wave
    DS3231_SquareWave1kHz = 0x08, // 1kHz square wave
    DS3231_SquareWave4kHz = 0x10, // 4kHz square wave
    DS3231_SquareWave8kHz = 0x18  // 8kHz square wave
};

/**************************************************************************/
/*!
    @brief  RTC based on the DS3231 chip connected via I2C and the Wire library
*/
/**************************************************************************/
class RTC_DS3231 {
public:
    bool begin(void);
    static void adjust(const DateTime& dt);
    bool lostPower(void);
    static DateTime now();
    static Ds3231SqwPinMode readSqwPinMode();
    static void writeSqwPinMode(Ds3231SqwPinMode mode);
    static float getTemperature();  // in Celcius degree
    static uint16_t getTemperatureBytes();
};

#endif // _RTCLIB_H_
