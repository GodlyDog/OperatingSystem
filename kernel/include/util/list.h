#pragma once

#include "kernel.h"

/*
 * Generic circular doubly linked list implementation.
 *
 * list_t is the head of the list.
 * list_link_t should be included in structures which want to be
 * linked on a list_t.
 *
 * All of the list functions take pointers to list_t and list_link_t
 * types, unless otherwise specified.
 *
 * list_init(list) initializes a list_t to an empty list.
 *
 * list_empty(list) returns 1 iff the list is empty.
 *
 * Insertion functions.
 *   list_insert_head(list, link) inserts link at the front of the list.
 *   list_insert_tail(list, link) inserts link at the end of the list.
 *   list_insert_before(olink, nlink) inserts nlink before olink in list.
 *
 * Removal functions.
 * Head is list->l_next.  Tail is list->l_prev.
 * The following functions should only be called on non-empty lists.
 *   list_remove(link) removes a specific element from the list.
 *   list_remove_head(list) removes the first element of list.
 *   list_remove_tail(list) removes the last element of list.
 *
 * Item accessors.
 *   list_item(link, type, member)
 *   
 *   Given a list_link_t* and the name of the type of structure which contains
 *   the list_link_t and the name of the member corresponding to the list_link_t,
 *   returns a pointer (of type "type*") to the item.
 *   
 *   Example: 
 *     struct my_struct { list_link_t my_link };
 *     struct my_struct a;
 *     list_link_init(&a.my_link);
 *   
 *     struct my_struct *b = list_item(&a.my_link, struct my_struct, my_link);
 *     // b should equal &a here
 *
 * To iterate over a list,
 *    list_link_t *link;
 *    for (link = list->l_next;
 *         link != list; link = link->l_next)
 *       ...
 *
 * Or, use the macros, which will work even if you list_remove() the
 * current link:
 *    list_iterate(list, iterator, type, member) {
 *        ... use iterator ...
 *    }
 *    (see also list_iterate_reverse for iterating in reverse)
 * 
 *    Where:
 *      - list is a pointer to the list_t to iterate over,
 *      - iterator is a name for the loop variable which will take on the value 
 *       of each item in the list,
 *      - type is the type of items in the list,
 *      - member is the name of the field in the item type that is the list_link_t
 * 
 *    Example (from kernel/drivers/chardev.c)
 *      // chardevs is a list_t
 *      // chardev_t has a cd_link member which is a list_link_t
 *      list_iterate(&chardevs, cd, chardev_t, cd_link)
 *      {
 *          if (dev->cd_id == cd->cd_id)
 *          {
 *              return -1;
 *          }
 *      }
 */

/**
 * Initialize a list_t.
 */
#define LIST_INITIALIZER(list)               \
    {                                        \
        .l_next = &(list), .l_prev = &(list) \
    }

/**
 * Initialize a list link.
 */
#define LIST_LINK_INITIALIZER(list_link) \
    {                                    \
        .l_next = NULL, .l_prev = NULL   \
    }

typedef struct list
{
    struct list *l_next;
    struct list *l_prev;
} list_t, list_link_t;

/**
 * Initialize a list link.
 */
void list_link_init(list_link_t *link);

/**
 * Initialize a list_t.
 */
void list_init(list_t *list);

/**
 * Check if a link is linked to some list.
 * 
 * @param link The link to check.
 * @return long 1 if linked, 0 otherwise.
 */
long list_link_is_linked(const list_link_t *link);

/**
 * Check if a list is empty.
 * 
 * @param list The list to check.
 * @return long 1 if empty, 0 otherwise.
 */
long list_empty(const list_t *list);

/**
 * Assert that the internal state of a list is sane, and 
 * panic if it is not.
 * 
 * @param list The list to check for sanity.
 */
void list_assert_sanity(const list_t *list);

/**
 * Insert a new link onto a list before another link.
 * 
 * @param link The link before which the new link should be inserted.
 * @param to_insert The new link to be inserted.
 */
void list_insert_before(list_link_t *link, list_link_t *to_insert);

/**
 * Insert a new link at the head (beginning) of a given list.
 * 
 * @param list The list to insert on.
 * @param link The new link to insert.
 */
void list_insert_head(list_t *list, list_link_t *link);

/**
 * Insert a new link at the tail (end) of a given list.
 * 
 * @param list The list to insert on.
 * @param link The new link to insert.
 */
void list_insert_tail(list_t *list, list_link_t *link);

/**
 * Remove a particular link from the list it's on.
 * 
 * @param link The link to be removed from its list.
 */
void list_remove(list_link_t *link);

/**
 * Get a pointer to the item that contains the given link. 
 * 
 * For instance, given a list_link_t contained within a proc_t, get a reference 
 * to the proc_t itself.
 * 
 * @param link The link contained within the item to access.
 * @param type The type of the outer item struct (e.g., proc_t)
 * @param member The name of the struct member which is the list_link_t (e.g. p_list_link)
 * 
 */
#define list_item(link, type, member) \
    (type *)((char *)(link)-offsetof(type, member))

/**
 * Get the item at the head of the list. See list_item for explanation
 * of type and member.
 */
#define list_head(list, type, member) list_item((list)->l_next, type, member)

/**
 * Get the item at the tail of the list. See list_item for explanation
 * of type and member.
 */
#define list_tail(list, type, member) list_item((list)->l_prev, type, member)

/**
 * Get the next item in a list that occurs after the given item.
 * 
 * @param current An item from the list (e.g. a proc_t)
 * See list_item for explanation of type and member.
 */
#define list_next(current, type, member) \
    list_head(&(current)->member, type, member)

/**
 * Get the previous item in a list given an item. See list_next for explanation.
 */
#define list_prev(current, type, member) \
    list_tail(&(current)->member, type, member)

/**
 * Iterate over elements in in a list. See comment at top of list.h for 
 * detailed description.
 */
#define list_iterate(list, var, type, member)               \
    for (type *var = list_head(list, type, member),         \
              *__next_##var = list_next(var, type, member); \
         &var->member != (list);                            \
         var = __next_##var, __next_##var = list_next(var, type, member))

/**
 * Iterate over the elements of a list in reverse. See comment at top of list.h for 
 * detailed description.
 */
#define list_iterate_reverse(list, var, type, member)       \
    for (type *var = list_tail(list, type, member),         \
              *__next_##var = list_prev(var, type, member); \
         &var->member != (list);                            \
         var = __next_##var, __next_##var = list_prev(var, type, member))
