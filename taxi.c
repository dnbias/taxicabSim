#include "taxi.h"
void incTrafficAt(Point p) {
  /*wait mutex*/
  mapptr[position.x][position.y]->Traffic++;
  /*signal mutex*/
}

void moveTo(Point p) { /*pathfinding*/
}

int main(int argc, char **argv) {
  int shmid, qid;
  key_t shmkey, qkey;
  void *mapptr;
  Point position;
  Message msg;
  external int executing;
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
  if ((qkey = ftok("makefile", 'd')) < 0) {
    EXIT_ON_ERROR
  }
  if ((qid = msgget(qkey, 0644)) < 0) {
    EXIT_ON_ERROR
  }
  signal(SIGINT, SIGINThandler);
  logmsg("Init position");
  sscanf(argv[1], "%d", &position.x);
  sscanf(argv[2], "%d", &position.y);

  incTrafficAt(position);

  while (executing) {
    msgrcv(qid, &msg, sizeof(Point), 0, 0);
    logmsg("Going to Source " + msg.type);
    moveTo(msg.source);
    logmsg("Going to destination");
    moveTo(msg.destination);
  }

  logmsg("Finishing up");
  shmdt(mapptr);
  exit(0);
}
