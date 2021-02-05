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
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifndef DEBUG /* DEBUG by setting when compiling -DDEBUG=1 */
#define DEBUG 0
#endif

#define SO_WIDTH 60 /* a tempo di compilazione */
#define SO_HEIGHT 20
#define EXIT_ON_ERROR                                                          \
  if (errno) {                                                                 \
    fprintf(stderr, "%d: pid %ld; errno: %d (%s)\n", __LINE__, (long)getpid(), \
            errno, strerror(errno));                                           \
    raise(SIGINT);                                                             \
  }

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

enum type { FREE, SOURCE, HOLE };

enum Level { RUNTIME, DB, SILENCE };

typedef struct {
  enum type state;
  int capacity;
  int traffic;
  int visits;
} Cell;

typedef struct {
  int SO_TAXI, SO_SOURCES, SO_HOLES, SO_TOP_CELLS, SO_CAP_MIN, SO_CAP_MAX,
      SO_TIMENSEC_MIN, SO_TIMENSEC_MAX, SO_TIMEOUT, SO_DURATION;
} Config;

typedef struct {
  int x, y;
} Point;

typedef struct {
  long type;
  Point destination;
} Message;

typedef struct {
  long type;
  int requests;
} MasterMessage;

int isFree(Cell (*map)[][SO_HEIGHT], Point p);

void semWait(Point, int);

void semSignal(Point, int);

void semSync(int);

void lock(int);

void unlock(int);

union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
  struct seminfo *__buf;
};

#endif /* __GENERAL_H_ */
