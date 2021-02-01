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

/*int sourceFree(int sem, int n){
  struct sembuf buf;
  buf.sem_num = n;
  buf.sem_op = -1;
  buf.sem_flg = IPC_NOWAIT;
  return semop(sem, &buf, 1);
}
void sourceSetFree(int sem, int n){
  struct sembuf buf;
  buf.sem_num = n;
  buf.sem_op = 1;
  buf.sem_flg = 0;
  if(semop(sem, &buf, 1) < 0)
    EXIT_ON_ERROR
}*/

void semWait(Point p, int sem) {
  struct sembuf buf;
  buf.sem_num = p.y * SO_WIDTH + p.x;
  buf.sem_op = -1;
  buf.sem_flg = SEM_UNDO;
  if(semop(sem, &buf, 1) < 0)
    EXIT_ON_ERROR
}

void semSignal(Point p, int sem) {
  struct sembuf buf;
  buf.sem_num = p.y * SO_WIDTH + p.x;
  buf.sem_op = 1;
  buf.sem_flg = SEM_UNDO;
  if(semop(sem, &buf, 1) < 0)
    EXIT_ON_ERROR
}

void semSync(int sem){
  struct sembuf buf;
  buf.sem_num = 0;
  buf.sem_op = 0;
  buf.sem_flg = 0;
  if(semop(sem, &buf, 1) < 0)
    EXIT_ON_ERROR
}
