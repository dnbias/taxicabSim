#include "general.h"

/* Checks whether the Point is Free */
int isFree(Cell (*map)[][SO_HEIGHT], Point p) {
  int r;
  if ((*map)[p.x][p.y].state == FREE) {
    r = 1;
  } else {
    r = 0;
  }
  return r;
}
