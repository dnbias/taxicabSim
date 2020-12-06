#ifndef __GENERATOR_H_
#define __GENERATOR_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>   // rand
#define SO_WIDTH 4  // a tempo di compilazione
#define SO_HEIGHT 4 // ^

enum type { FREE, SOURCE, HOLE };

typedef struct {
  enum type state;
  int capacity;
  int traffic;
  int visits;
} Cell;

typedef struct {
  int SO_TAXI, SO_SOURCES, SO_HOLES, SO_CAP_MIN, SO_CAP_MAX, SO_TIMENSEC_MIN,
      SO_TIMENSEC_MAX, SO_TIMEOUT, SO_DURATION;
} Config;

#endif // __GENERATOR_H_
