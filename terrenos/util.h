
#pragma once

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <libnoise/noise.h>

#include "../libmt/libmt.h"

class Task {
private:
  timespec start;
  static uint8_t& Depth() {static uint8_t d = 0; return d;}

public:
  template <typename...ARGS> Task(ARGS...args) {
    int indent = Depth()++;
    while (indent--) printf("  ");

    printf(args...);
    printf("...");
    fflush(stdout);

    clock_gettime(CLOCK_MONOTONIC, &start);
  }
 
  ~Task() {
    timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);

    printf(" %.3fms\n", (end.tv_sec-start.tv_sec)*1e3 + (end.tv_nsec-start.tv_nsec)*1e-6);
    Depth()--;
  }
};



enum HeightType {
	HT_High = 0x00,
	HT_Mid = 0x01,
	HT_Low = 0x02,
	HT_Water = 0x03
};

