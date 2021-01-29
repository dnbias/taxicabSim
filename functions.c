#include "general.h"

/* Checks whether the Point is Free */
int isFree(Cell (*map)[][SO_HEIGHT], Point p) {
  int r;
  if (leggi(p)) {
    printf("error leggi \n");
    EXIT_ON_ERROR;
  }
  if ((*map)[p.x][p.y].state != HOLE &&
      (*map)[p.x][p.y].traffic < (*map)[p.x][p.y].capacity) {
    if (releaseR(p)) {
      printf("releaseR \n");
      EXIT_ON_ERROR;
    }
    if (DEBUG)
      printf("[pid::%d] Point (%d,%d) is free\n", getpid(), p.x, p.y);
    r = 1;
  } else {
    if (DEBUG)
      printf("[pid::%d] Point (%d,%d) is not free\n", getpid(), p.x, p.y);
    r = 0;
  }
  return r;
}
