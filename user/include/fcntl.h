/* fcntl.h - File access bits
 * mcc, jal
 */

#pragma once

/* Kernel and user header (via symlink) */

/* File access modes for open(). */
#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_ACCESSMODE_MASK (O_RDONLY | O_WRONLY | O_RDWR)

/* File status flags for open(). */
#define O_CREAT 0x100  /* Create file if non-existent. */
#define O_TRUNC 0x200  /* Truncate to zero length. */
#define O_APPEND 0x400 /* Append to file. */
