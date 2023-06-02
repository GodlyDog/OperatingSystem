
#include <util/debug.h>
#include <util/list.h>

inline void list_init(list_t *list) { list->l_next = list->l_prev = list; }

inline void list_link_init(list_link_t *link)
{
    link->l_next = link->l_prev = NULL;
}

inline long list_link_is_linked(const list_link_t *link)
{
    return link->l_next && link->l_prev;
}

inline long list_empty(const list_t *list) { return list->l_next == list; }

inline void list_assert_sanity(const list_t *list)
{
    KASSERT(list->l_next && list->l_next->l_prev && list->l_prev &&
            list->l_prev->l_next);
}

inline void list_insert_before(list_link_t *link, list_link_t *to_insert)
{
    list_link_t *prev = to_insert;
    list_link_t *next = link;
    prev->l_next = next;
    prev->l_prev = next->l_prev;
    next->l_prev->l_next = prev;
    next->l_prev = prev;
}

inline void list_insert_head(list_t *list, list_link_t *link)
{
    list_insert_before((list)->l_next, link);
}

inline void list_insert_tail(list_t *list, list_link_t *link)
{
    list_insert_before(list, link);
}

inline void list_remove(list_link_t *link)
{
    list_link_t *ll = link;
    list_link_t *prev = ll->l_prev;
    list_link_t *next = ll->l_next;
    prev->l_next = next;
    next->l_prev = prev;
    ll->l_next = ll->l_prev = NULL;
}
