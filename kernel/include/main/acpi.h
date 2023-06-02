#pragma once

#include <types.h>

typedef struct acpi_header
{
    uint32_t ah_sign;
    uint32_t ah_size;
    uint8_t ah_rev;
    uint8_t ah_checksum;
    uint8_t ah_oemid[6];
    uint8_t ah_tableid[8];
    uint32_t ah_oemrev;
    uint32_t ah_creatorid;
    uint32_t ah_creatorrev;
} packed acpi_header_t;

void acpi_init();

void *acpi_table(uint32_t signature, int index);
