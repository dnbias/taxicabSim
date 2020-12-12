#include "taxi.h"

int main(int argc, char **argv) {
  int shmid;
  Cell map[SO_WIDTH][SO_HEIGHT];
  shmid = shmget(IPC_PRIVATE, SO_WIDTH * SO_HEIGHT * sizeof(Cell), 0666);
  if (shmid < 0) {
    printf("shmget error\n");
    EXIT_ON_ERROR
  }
  shmat(shmid, &map, 0);
  printMap(&map);

  logmsg("Finishing up");
  shmdt(&map);
  exit(0);
}
