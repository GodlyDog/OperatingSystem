#pragma once

#include "mm/mobj.h"
#include "proc/kmutex.h"
#include "types.h"

typedef struct pframe
{
    size_t pf_pagenum;
    void *pf_addr;
    long pf_dirty;
    kmutex_t pf_mutex;
    list_link_t pf_link;
} pframe_t;

void pframe_init();

pframe_t *pframe_create();

void pframe_release(pframe_t **pfp);

void pframe_free(pframe_t **pfp);
