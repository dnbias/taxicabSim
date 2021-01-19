#include "general.h"

/* Checks whether the Point is Free */
int isFree(Cell (*map)[][SO_HEIGHT], Point p) {
  int r;
  /*
  if(leggi(p)){
        EXIT_ON_ERROR;
  }
  */
  if ((*map)[p.x][p.y].state == FREE && (*map)[p.x][p.y].traffic < (*map)[p.x][p.y].capacity) {
    /*
        if(release(p)){
                EXIT_ON_ERROR;
                }*/
    r = 1;
  } else {
    r = 0;
  }
  return r;
}
