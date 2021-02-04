#include "general.h"

/* Checks whether the Point is Free */
int isFree(Cell (*map)[][SO_HEIGHT], Point p) {
  int r;
  if ((*map)[p.x][p.y].state != HOLE) {
    r = 1;
  } else {
    r = 0;
  }
  return r;
}

void semWait(Point p, int sem) {
  struct sembuf buf;
  buf.sem_num = p.y * SO_WIDTH + p.x;
  buf.sem_op = -1;
  buf.sem_flg = SEM_UNDO;
  if (semop(sem, &buf, 1) < 0) {
    logmsg("Failed semWait", DB);
    kill(getppid(), SIGUSR1);
    raise(SIGINT);
  }
}

void semSignal(Point p, int sem) {
  struct sembuf buf;
  buf.sem_num = p.y * SO_WIDTH + p.x;
  buf.sem_op = 1;
  buf.sem_flg = SEM_UNDO;
  if (semop(sem, &buf, 1) < 0) {
    logmsg("Failed semSignal", DB);
    kill(getppid(), SIGUSR1);
    raise(SIGINT);
  }
}

void semSync(int sem) {
  struct sembuf buf;
  buf.sem_num = 0;
  buf.sem_op = 0;
  buf.sem_flg = 0;
  if (semop(sem, &buf, 1) < 0) {
    kill(getppid(), SIGUSR1);
    raise(SIGINT);
  }
}

void lock(int sem) {
  struct sembuf buf;
  buf.sem_num = 0;
  buf.sem_op = -1;
  buf.sem_flg = SEM_UNDO;
  if (semop(sem, &buf, 1) < 0) {
    logmsg("Failed lock", DB);
    kill(getppid(), SIGUSR1);
    raise(SIGINT);
  }
}
void unlock(int sem) {
  struct sembuf buf;
  buf.sem_num = 0;
  buf.sem_op = 1;
  buf.sem_flg = SEM_UNDO;
  if (semop(sem, &buf, 1) < 0) {
    logmsg("Failed unlock", DB);
    kill(getppid(), SIGUSR1);
    raise(SIGINT);
  }
}
