#include "taxi.h"
#include "general.h"
#include <stdio.h>
#include <unistd.h>

void *mapptr;
Point position;
int *executing, qid;

void SIGINThandler(int sig) {
  *executing = 0;
  logmsg("Finishing up");
  shmdt(mapptr);
  shmdt(executing);
  logmsg("Graceful exit successful");
  exit(0);
}

void incTrafficAt(Point p) {
  /*wait mutex*/
  ((Cell(*)[SO_WIDTH][SO_HEIGHT])mapptr)[p.x][p.y]->traffic++;
  logmsg("Incrementato traffico in ");
  printf("\t(%d,%d)\n", p.x, p.y);
  /*signal mutex*/
}

void moveTo(Point p) { /*pathfinding*/
  /* funzione nanosleep permette di aspettare per il tempo indicato da */
  usleep(5000000);
  logmsg("Moving to");
  printf("\t(%d,%d)\n", p.x, p.y);
  incTrafficAt(p);
}

int main(int argc, char **argv) {
  int shmid;
  key_t shmkey, qkey;
  Message msg;

  logmsg("Inizializzazione");
  sleep(1);

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
  logmsg("Init position");
  sscanf(argv[1], "%d", &position.x);
  sscanf(argv[2], "%d", &position.y);
  /************END-INIT************/

  incTrafficAt(position);
  sleep(3);

  while (*executing) {
    msgrcv(qid, &msg, sizeof(Point), 0, 0);
    logmsg("Going to Nearest Source");
    /* moveTo(getNearSource()); ********** TODO *********/
    logmsg("Going to destination");
    moveTo(msg.destination);
  }
}
