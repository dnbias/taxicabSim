#ifndef __GENERAL_H_
#define __GENERAL_H_
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h> /* rand */
#include <unistd.h>
#define SO_WIDTH 4 /* a tempo di compilazione */
#define SO_HEIGHT 4
#define MAX_SOURCES (SO_WIDTH * SO_HEIGHT / 3)
#define EXIT_ON_ERROR                                                          \
  if (errno) {                                                                 \
    fprintf(stderr, "%d: pid %ld; errno: %d (%s)\n", __LINE__, (long)getpid(), \
            errno, strerror(errno));                                           \
    exit(EXIT_FAILURE);                                                        \
  }

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

typedef struct {
  int x, y;
} Point;

typedef struct {
  long type;
  Point source;
  Point destination;
} Message;

/*
 * Print on stdout the map in a readable format:
 *     FREE Cells are printed as   [ ]
 *     SOURCE Cells are printed as [S]
 *     HOLE Cells are printed as   [H]
 */
void printMap(Cell (*map)[SO_WIDTH][SO_HEIGHT]) {
  int x, y;
  for (y = 0; y < SO_HEIGHT; y++) {
    for (x = 0; x < SO_WIDTH; x++) {
      switch (map[x][y]->state) {
      case FREE:
        printf("[ ]");
        break;
      case SOURCE:
        printf("[S]");
        break;
      case HOLE:
        printf("[X]");
      }
    }
    printf("\n");
  }
}

#endif /* __GENERAL_H_ */
