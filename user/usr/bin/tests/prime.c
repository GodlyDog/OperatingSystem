#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef uint64_t bitmask_word_t;
#define BITMASK_WORD_BITWIDTH 64
#define BITMASK_WORD_BITWIDTH_LOG2 6
#define BITMASK_IDX(x) ((x) >> BITMASK_WORD_BITWIDTH_LOG2)
#define BITMASK_POS(x) ((x) & ~(~0UL << BITMASK_WORD_BITWIDTH_LOG2))

// 7  6  5  4  3  2  1  0 | 15 14 13 12 11 10 9  8
#define BITMASK_POS_MASK(x) (1UL << BITMASK_POS(x))

#define BITMASK_MAX_IDX(x) (BITMASK_IDX((x)-1UL) + 1UL)
#define BITMASK_SIZE(x) (sizeof(bitmask_word_t) * BITMASK_MAX_IDX(x))
#define GET_BIT(bitmask, x) \
    (((bitmask_word_t *)bitmask)[BITMASK_IDX(x)] & BITMASK_POS_MASK(x))
#define SET_BIT(bitmask, x) \
    (((bitmask_word_t *)bitmask)[BITMASK_IDX(x)] |= BITMASK_POS_MASK(x))
#define UNSET_BIT(bitmask, x) \
    (((bitmask_word_t *)bitmask)[BITMASK_IDX(x)] &= ~BITMASK_POS_MASK(x))

inline uint64_t *find_ne64(const uint64_t not, const uint64_t *start,
                           size_t count)
{
    uint64_t *ret;
    __asm__ volatile(
        "cld; repe; scasq; xor %%rax, %%rax; setz %%al; add %%rax, %%rcx;"
        : "=D"(ret), "=c"(count)
        : "A"(not), "D"(start), "c"(count)
        : "cc");
    return count ? (ret - 1) : NULL;
}

size_t next_set_bit(const bitmask_word_t *bitmask, size_t start,
                    size_t max_idx)
{
    size_t idx = BITMASK_IDX(start);
    bitmask_word_t copy = (bitmask[idx++] >> BITMASK_POS(start)) - 1;
    if (copy)
    {
        return start + __builtin_ctzl(copy);
    }
    if (idx < max_idx)
    {
        uint64_t *end = find_ne64(0, bitmask + idx, max_idx - idx);
        if (end)
            return (((end - bitmask) << BITMASK_WORD_BITWIDTH_LOG2) +
                    (unsigned)__builtin_ctzl(*end));
    }
    return ~0UL;
}

long compute_largest_prime(long n)
{
    if (n <= 1)
        return -1;
    if (n <= 3)
        return n;
    n = (n - 1) >> 1;

    bitmask_word_t *bitmask =
        malloc(BITMASK_SIZE(n)); // GET_BIT(i) --> is 2 * i + 1 prime?
    size_t max_idx = BITMASK_MAX_IDX(n);
    memset(bitmask, 0xff, BITMASK_SIZE(n));

    UNSET_BIT(bitmask, 0);
    size_t prime_idx = 1;

    while (1)
    {
        long increment = (prime_idx << 1) | 1; // prime_idx * 2 + 1
        for (long multiple = prime_idx + increment; multiple <= n;
             multiple += increment)
        {
            UNSET_BIT(bitmask, multiple);
        }
        //        size_t next_prime_idx = prime_idx + 1;
        //        while (!GET_BIT(bitmask, next_prime_idx) && next_prime_idx <=
        //        n)
        //            next_prime_idx++;

        size_t next_prime_idx = next_set_bit(bitmask, prime_idx, max_idx);

        if (next_prime_idx > (size_t)n)
            break;
        prime_idx = next_prime_idx;
    }

    free(bitmask);
    return (prime_idx << 1) | 1;
}

int main(int argc, char *argv[], char *envp[])
{
    if (argc <= 1)
    {
        fprintf(stderr,
                "USAGE: \"prime <n>\" to compute the largest prime <= n\n");
        exit(1);
    }
    long n = strtol(argv[1], NULL, 0);
    fprintf(stdout, "%ld\n", compute_largest_prime(n));
    return 0;
}
