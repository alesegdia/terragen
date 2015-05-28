#pragma once
#include <stdint.h>

namespace SFMT607 {
#define SFMT_MEXP 607
#include "SFMT-src-1.4.1/SFMT.h"
}

class MT {
private:
  SFMT607::sfmt_t sfmt;
  uint32_t bits_left;
  uint8_t  bits_count;

public:
  MT(uint32_t seed) : bits_left(0) {SFMT607::sfmt_init_gen_rand(&sfmt, seed);}

  uint32_t u32() {return SFMT607::sfmt_genrand_uint32(&sfmt);}
  int32_t  s32() {return (int32_t)SFMT607::sfmt_genrand_uint32(&sfmt);}
  uint64_t u64() {return SFMT607::sfmt_genrand_uint64(&sfmt);}
  int64_t  s64() {return (int64_t)SFMT607::sfmt_genrand_uint64(&sfmt);}
  uint32_t bits(int n);

/*
  void operator() (uint32_t&n) {n = sfmt_genrand_uint32(&sfmt);}
  void operator() (int32_t&n) {n = (int32_t)sfmt_genrand_uint32(&sfmt);}
  void operator() (uint64_t&n) {n = sfmt_genrand_uint64(&sfmt);}
  void operator() (int64_t&n) {n = (int32_t)sfmt_genrand_uint64(&sfmt);}
*/
};