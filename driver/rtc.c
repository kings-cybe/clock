/*
 * MIT License
 *
 * Copyright (c) 2023 梅花嵌入式
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "stm32f10x.h"
#include "rtc.h"


static bool date_validate(const rtc_date_t *date)
{
    if (date->year < 1970 || date->year > 2099)
        return false;
    if (date->month < 1 || date->month > 12)
        return false;
    if (date->day < 1 || date->day > 31)
        return false;
    if (date->hour > 23)
        return false;
    if (date->minute > 59)
        return false;
    if (date->second > 59)
        return false;

    return true;
}

uint32_t date_to_ts(const rtc_date_t *date)
{
    uint16_t year = date->year;
    uint8_t month = date->month;
    uint8_t day = date->day;
    uint8_t hour = date->hour;
    uint8_t minute = date->minute;
    uint8_t second = date->second;

    uint64_t ts = 0;

    /* 二月闰月，计算当作最后一个月份 */
    month -= 2;
    if ((int8_t)month <= 0)
    {
        month += 12;
        year -= 1;
    }

    /* 计算时间戳 */
    ts = (((year / 4 - year / 100 + year / 400 + 367 * month / 12 + day + year * 365 - 719499) * 24 +
          hour) * 60 + minute) * 60 + second;

    return ts;
}

void ts_to_date(uint32_t seconds, rtc_date_t *date)
{
    uint32_t leapyears = 0, yearhours = 0;
    const uint32_t mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    const uint16_t ONE_YEAR_HOURS = 8760;

    memset(date, 0, sizeof(rtc_date_t));

    /* 秒 */
    date->second = seconds % 60;
    seconds /= 60;

    /* 分 */
    date->minute = seconds % 60;
    seconds /= 60;

    /* 年 */
    leapyears = seconds / (1461 * 24);
    date->year = (leapyears << 2) + 1970;
    seconds %= 1461 * 24;

    for (;;)
    {
        yearhours = ONE_YEAR_HOURS;

        /* 闰年加一天 */
        if ((date->year & 3) == 0)
            yearhours += 24;

        /* 剩余时间已经不足一年了跳出循环 */
        if (seconds < yearhours)
            break;

        date->year++;
        /* 减去一年的时间 */
        seconds -= yearhours;
    }

    /* 时 */
    date->hour = seconds % 24;
    seconds /= 24;
    seconds++;

    /* 闰年 */
    if ((date->year & 3) == 0)
    {
        if (seconds > 60)
        {
            seconds--;
        }
        else
        {
            if (seconds == 60)
            {
                date->month = 2;
                date->day = 29;
                return;
            }
        }
    }

    /* 月 */
    for (date->month = 0; mdays[date->month] < seconds; date->month++)
        seconds -= mdays[date->month];
    date->month++;

    /* 日 */
    date->day = seconds;
}

void rtc_init(void)
{
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    RCC_RTCCLKCmd(ENABLE);
    RTC_WaitForSynchro();
    RTC_WaitForLastTask();
    RTC_SetPrescaler(32767);
    RTC_WaitForLastTask();
}

void rtc_set_date(rtc_date_t *date)
{
    if (!date_validate(date))
        return;

    uint32_t ts = date_to_ts(date);

    RTC_WaitForLastTask();
    RTC_SetCounter(ts);
    RTC_WaitForLastTask();
}

void rtc_get_date(rtc_date_t *date)
{
    uint32_t ts = RTC_GetCounter();

    if (date)
    {
        ts_to_date(ts, date);
    }
}

void rtc_set_timestamp(uint32_t timestamp)
{
    RTC_WaitForLastTask();
    RTC_SetCounter(timestamp);
    RTC_WaitForLastTask();
}

void rtc_get_timestamp(uint32_t *timestamp)
{
    if (timestamp)
    {
        *timestamp = RTC_GetCounter();
    }
}
