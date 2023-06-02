#pragma once

// random macro for multiboot header
#define TAG_SIZE(x) (((x)-1) / MULTIBOOT_TAG_ALIGN + 1)

extern struct multiboot_tag *mb_tag;
