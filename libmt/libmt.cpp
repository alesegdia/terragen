#include "libmt.h"

namespace SFMT607 {
#include "SFMT-src-1.4.1/SFMT.c"
}

uint32_t MT::bits(int n)
{
  if (!bits_count) {
    bits_left = sfmt_genrand_uint32(&sfmt);
    bits_count = 32;
  }
  
  if (n <= bits_count) {
    uint32_t r = bits_left & ((1<<n)-1);
    bits_left >>= n;
    bits_count -= n;
    return r;
  }

  uint8_t remaining = n-bits_count;
  uint32_t r = bits_left << remaining;
  bits_count = 0;
  return r | bits(remaining);
}

