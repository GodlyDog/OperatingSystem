#pragma once

/* Kernel and user header (via symlink) */

#define NULL 0

#define packed __attribute__((packed))

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;

typedef signed long int64_t;
typedef unsigned long uint64_t;
typedef signed long intptr_t;
typedef unsigned long uintptr_t;
typedef uint64_t size_t;
typedef int64_t ssize_t;
typedef int64_t off_t;

typedef int32_t pid_t;
typedef uint16_t mode_t;
typedef uint32_t blocknum_t;
typedef uint32_t ino_t;
typedef uint32_t devid_t;

typedef uint64_t time_t;
typedef uint64_t useconds_t;