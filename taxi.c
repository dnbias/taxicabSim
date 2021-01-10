#include "taxi.h"

void *mapptr;
Point position;
int *executing, qid;

int main(int argc, char **argv) {
  int shmid;
  key_t shmkey, qkey;
  Message msg;

  logmsg("Init...", DB);
  if (DEBUG) sleep(1);

  /************INIT************/
  if ((shmkey = ftok("makefile", 'a')) < 0) {
    EXIT_ON_ERROR
  }

  if ((shmid = shmget(shmkey, sizeof(int), 0644)) < 0) {
    EXIT_ON_ERROR
  }

  if ((executing = shmat(shmid, NULL, 0)) < (int *)0) {
    EXIT_ON_ERROR
  }
  *executing = 1;

  if ((shmkey = ftok("makefile", 'd')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((shmid = shmget(shmkey, 0, 0644)) < 0) {
    EXIT_ON_ERROR
  }
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
  sscanf(argv[1], "%d", &position.x);
  sscanf(argv[2], "%d", &position.y);
  logmsg("Init Finished", DB);
  /************END-INIT************/

  incTrafficAt(position);

  while (*executing) {
    msgrcv(qid, &msg, sizeof(Point), 0, 0);
    logmsg("Going to Nearest Source", DB);
    /* moveTo(getNearSource()); ********** TODO *********/
    logmsg("Going to destination", DB);
    moveTo(msg.destination);
  }
}

void moveTo(Point p) { /*pathfinding*/
  if (DEBUG) usleep(5000000);
  logmsg("Moving to", DB);
  if (DEBUG) printf("\t(%d,%d)\n", p.x, p.y);
  incTrafficAt(p);
}

void incTrafficAt(Point p) {
  /*wait mutex*/
  ((Cell(*)[SO_WIDTH][SO_HEIGHT])mapptr)[p.x][p.y]->traffic++;
  logmsg("Incrementato traffico in", DB);
  if(DEBUG) printf("\t(%d,%d)\n", p.x, p.y);
  /*signal mutex*/
}

void logmsg(char *message, enum Level l) {
  if(l <= DEBUG){
    printf("[taxi-%d] %s\n", getpid(), message);
  }
}

void SIGINThandler(int sig) {
  /* *executing = 0; */
  logmsg("Finishing up", DB);
  shmdt(mapptr);
  shmdt(executing);
  logmsg("Graceful exit successful", DB);
  exit(0);
}
