#include "general.h"

/* Checks whether the Point is Free */
int isFree(Cell (*map)[][SO_HEIGHT], Point p) {
  int r;
  if ((*map)[p.x][p.y].state == FREE &&
      ((*map)[p.x][p.y].traffic < (*map)[p.x][p.y].capacity)) {
    r = 0;
  } else {
    r = 1;
  }
  return r;
}
