#include "../../include/comunicazione/thread_safe_rand.h"

#include <time.h>

//! PRIVATE FUNCTION

static int32_t temper(int32_t x)
{
    x ^= x >> 11;
    x ^= x << 7 & 0x9D2C5680;
    x ^= x << 15 & 0xEFC60000;
    x ^= x >> 18;
    return x;
}

//! PUBLIC FUNCTION

int32_t generate_random(int32_t seed)
{
    uint64_t random = time(NULL) ^ seed;
    random = 6364136223846793005ULL * random + 1;
    return temper(random >> 32);
}