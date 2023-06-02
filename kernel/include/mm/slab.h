#pragma once

#include <types.h>

/* Define SLAB_REDZONE to add top and bottom redzones to every object. */
#define SLAB_REDZONE 0xdeadbeefdeadbeef

/* Define SLAB_CHECK_FREE to add extra book keeping to make sure there
 * are no double frees. */
#define SLAB_CHECK_FREE

/*
 * The slab allocator. A "cache" is a store of objects; you create one by
 * specifying a constructor, destructor, and the size of an object. The
 * "alloc" function allocates one object, and the "free" function returns
 * it to the free list *without calling the destructor*. This lets you save
 * on destruction/construction calls; the idea is that every free object in
 * the cache is in a known state.
 */
typedef struct slab_allocator slab_allocator_t;

/* Initializes the slab allocator subsystem. This should be done
 * only after the page subsystem has been initialized. Slab allocators
 * and kmalloc will not work until this function has been called. */
void slab_init();

/*
 * Example Usage
 * See the below example for how to use a slab allocator to allocate objects 
 * of a given size. Note that you usually don't need to destroy most allocators,
 * as they should last as long as the system is running (e.g. the process allocator).
 * 
 * ```
 * typedef struct { 
 *     int x; 
 *     int y; 
 * } point_t;
 *
 * // Create a new allocator for objects of type point_t. This only needs to 
 * // happen once, usually in an initialization routine.
 * slab_allocator_t *point_allocator = slab_allocator_create("point", sizeof(point_t));
 * 
 * // Allocate a new point_t from the slab allocator
 * point_t *p = (point_t *)slab_obj_alloc(point_allocator);
 * 
 * // ... Use p here ...
 * 
 * // Deallocate the point_t
 * slab_obj_free(point_allocator, p);
 * ``` 
 */

/**
 * Creates a slab allocator for allocating objects of a given size.
 * 
 * @param name The name of the allocator (for debugging)
 * @param size The size (bytes) of objects that will be allocated from this allocator
 * @return slab_allocator_t* An allocator, or NULL on failure
 */
slab_allocator_t *slab_allocator_create(const char *name, size_t size);

/**
 * Destroys a slab allocator.
 * 
 * @param allocator The allocator to destroy
 */
void slab_allocator_destroy(struct slab_allocator *allocator);

/**
 * Allocates an object from the given slab allocator. The object is a chunk of 
 * memory as big as the size that slab allocator was created with. 
 * 
 * @param allocator The allocator to allocate from
 * @return void* A chunk of memory of the appropriate object size, or NULL
 *  on failure
 */
void *slab_obj_alloc(slab_allocator_t *allocator);

/**
 * Frees a given object that was allocated by a given slab allocator.
 * 
 * @param allocator The allocator that allocated this object
 * @param obj The object to be freed
 */
void slab_obj_free(slab_allocator_t *allocator, void *obj);

/**
 * Reclaims memory from unused slabs. 
 * 
 * NOTE: This is not currently implemented.
 * 
 * @param target Target number of pages to reclaim. If negative, reclaim as many
 *  as possible
 * @return long Number of pages freed
 */
long slab_allocators_reclaim(long target);