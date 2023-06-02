#ifndef ATOMIC_H
#define ATOMIC_H

typedef int atomic_t;

#define ATOMIC_INIT(i) (i)

static inline int __atomic_add_unless(atomic_t *a, int v, int u)
{
    int c, old;
    c = __sync_fetch_and_add(a, 0);
    while (c != u && (old = __sync_val_compare_and_swap(a, c, c + v)) != c)
        c = old;
    return c;
}

static inline void atomic_set(atomic_t *a, int i) { *a = i; }

static inline void atomic_inc(atomic_t *a) { __sync_add_and_fetch(a, 1); }

static inline int atomic_dec_and_test(atomic_t *a)
{
    return __sync_sub_and_fetch(a, 1) == 0;
}

static inline int atomic_inc_not_zero(atomic_t *a)
{
    return __atomic_add_unless(a, 1, 0);
}

#endif