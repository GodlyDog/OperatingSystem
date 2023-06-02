#ifndef CMOS_H
#define CMOS_H

#include "main/io.h"

// See: https://wiki.osdev.org/CMOS
#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

#define CMOS_REG_SECOND 0x00
#define CMOS_REG_MINUTE 0x02
#define CMOS_REG_HOUR 0x04
#define CMOS_REG_DAY 0x07
#define CMOS_REG_MONTH 0x08
#define CMOS_REG_YEAR 0x09

// We're on a modern computer. It'll have a century register.
#define CMOS_REG_CENTURY 0x32
#define CMOS_REG_STAT_A 0x0A
#define CMOS_REG_STAT_B 0x0B

typedef struct rtc_time_t
{
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned int year;

    // Internal use ONLY
    unsigned int __century;
} rtc_time_t;

unsigned char cmos_read_register(int reg);

/* Get the time from the CMOS RTC */
rtc_time_t rtc_get_time();

#endif