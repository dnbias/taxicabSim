#include "taxi.h"

int main(int argc, char **argv) {
  int shmid;
  key_t shmkey;
  void *mapptr;
  if ((shmkey = ftok("makefile", 'd')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  shmid = shmget(shmkey, 0, 0644);
  if (shmid < 0) {
    printf("shmget error\n");
    EXIT_ON_ERROR
  }
  if ((mapptr = shmat(shmid, NULL, 0)) < (void *)0) {
    EXIT_ON_ERROR
  }
  printMap(mapptr);

  logmsg("Finishing up");
  shmdt(mapptr);
  exit(0);
}
