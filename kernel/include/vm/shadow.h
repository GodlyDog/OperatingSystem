#pragma once

#include "mm/mobj.h"

void shadow_init();

mobj_t *shadow_create(mobj_t *shadowed);

void shadow_collapse(mobj_t *o);

extern int shadow_count;
