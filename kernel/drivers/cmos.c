#include "drivers/cmos.h"

int cmos_update_flag_set()
{
    outb(CMOS_ADDR, CMOS_REG_STAT_A);
    return (inb(CMOS_DATA) & 0x80);
}

unsigned char cmos_read_register(int reg)
{
    outb(CMOS_ADDR, reg);
    return inb(CMOS_DATA);
}

int rtc_time_match(rtc_time_t a, rtc_time_t b)
{
    return (a.second == b.second) && (a.minute == b.minute) &&
           (a.hour == b.hour) && (a.day == b.day) && (a.month == b.month) &&
           (a.year == b.year) && (a.__century == b.__century);
}

rtc_time_t __get_rtc_time()
{
    rtc_time_t tm;

    while (cmos_update_flag_set())
        ;

    tm.second = cmos_read_register(CMOS_REG_SECOND);
    tm.minute = cmos_read_register(CMOS_REG_MINUTE);
    tm.hour = cmos_read_register(CMOS_REG_HOUR);
    tm.day = cmos_read_register(CMOS_REG_DAY);
    tm.month = cmos_read_register(CMOS_REG_MONTH);
    tm.year = cmos_read_register(CMOS_REG_YEAR);
    tm.__century = cmos_read_register(CMOS_REG_CENTURY);

    return tm;
}

/* Our ticks -> time calculation is so suspect, we just get the time from the
 * CMOS RTC */
rtc_time_t rtc_get_time()
{
    // Check the result of CMOS twice to ensure we didn't get a torn read.
    rtc_time_t tm_a;
    rtc_time_t tm_b;

    do
    {
        tm_a = __get_rtc_time();
        tm_b = __get_rtc_time();
    } while (!rtc_time_match(tm_a, tm_b));

    unsigned char cmos_settings = cmos_read_register(CMOS_REG_STAT_B);

    // Convert from BCD
    if (!(cmos_settings & 0x04))
    {
        tm_a.second = (tm_a.second & 0x0F) + ((tm_a.second / 16) * 10);
        tm_a.minute = (tm_a.minute & 0x0F) + ((tm_a.minute / 16) * 10);
        tm_a.hour = ((tm_a.hour & 0x0F) + (((tm_a.hour & 0x70) / 16) * 10)) |
                    (tm_a.hour & 0x80);
        tm_a.day = (tm_a.day & 0x0F) + ((tm_a.day / 16) * 10);
        tm_a.month = (tm_a.month & 0x0F) + ((tm_a.month / 16) * 10);
        tm_a.year = (tm_a.year & 0x0F) + ((tm_a.year / 16) * 10);
        tm_a.__century = (tm_a.__century & 0x0F) + ((tm_a.__century / 16) * 10);
    }

    // Convert 12-hour clock to 24-hour clock:
    if (!(cmos_settings & 0x02) && (tm_a.hour & 0x80))
    {
        tm_a.hour = ((tm_a.hour & 0x7F) + 12) % 24;
    }

    tm_a.year += (tm_a.__century * 100);

    return tm_a;
}