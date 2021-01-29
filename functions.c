#include "general.h"

/* Checks whether the Point is Free */
int isFree(Cell (*map)[][SO_HEIGHT], Point p, int sem_idW, int sem_idR) {
  int r;

  if(leggi(p, sem_idW, sem_idR)){
  	    printf("error leggi \n");
        EXIT_ON_ERROR;
  }
  if ((*map)[p.x][p.y].state != HOLE &&
      (*map)[p.x][p.y].traffic < (*map)[p.x][p.y].capacity) {
      if(releaseR(p, sem_idR)){
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

int leggi(Point p, int sem_idR, int sem_idW) {
  struct sembuf writer, reader;
  writer.sem_num = p.y * SO_WIDTH + p.x;
  writer.sem_op = 0;
  writer.sem_flg = 0;
  reader.sem_num = p.y * SO_WIDTH + p.x;
  reader.sem_op = 1;
  reader.sem_flg = 0;
  return semop(sem_idW, &writer, 1) + semop(sem_idR, &reader, 1);
}

int scrivi(Point p, int sem_idR, int sem_idW) {
  /*semctl(sem_idW, p.y*SO_WIDTH + p.x, GETZCNT, &idR);*/
  struct sembuf writer[2], reader;
  writer[0].sem_num = p.y * SO_WIDTH + p.x;
  writer[0].sem_op = 0;
  writer[0].sem_flg = 0;
  reader.sem_num = p.y * SO_WIDTH + p.x;
  reader.sem_op = 0;
  reader.sem_flg = 0;
  writer[1].sem_num = p.y * SO_WIDTH + p.x;
  writer[1].sem_op = 1;
  writer[1].sem_flg = 0;
  return semop(sem_idW, writer, 1) + semop(sem_idR, &reader, 2);
}

int releaseW(Point p, int sem_idW) {
  struct sembuf releaseW;
  releaseW.sem_num = p.y * SO_WIDTH + p.x;
  releaseW.sem_op = -1;
  releaseW.sem_flg = IPC_NOWAIT;
  return semop(sem_idW, &releaseW, 1);
}

int releaseR(Point p, int sem_idR) {
  struct sembuf releaseR;
  releaseR.sem_num = p.y * SO_WIDTH + p.x;
  releaseR.sem_op = -1;
  releaseR.sem_flg = IPC_NOWAIT;
  return semop(sem_idR, &releaseR, 1);
}
