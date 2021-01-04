#include "general.h"
#include <stdio.h>
#include <unistd.h>
void *mapptr;
int *executing, qid;

void logmsg(char *message) {
  int pid;
  if(DEBUG){
    pid = getpid();
    printf("[source-%d] %s\n", pid, message);
  }
}

int isFree(Cell (*map)[SO_WIDTH][SO_HEIGHT], Point p) {
  int r;
  if (map[p.x][p.y]->state == FREE &&
      (map[p.x][p.y]->traffic < map[p.x][p.y]->capacity)) {
    r = 0;
  } else {
    r = 1;
  }
  return r;
}

void SIGINThandler(int sig) {
  /* *executing = 0; */
  logmsg("Finishing up");
  shmdt(mapptr);
  shmdt(executing);
  logmsg("Graceful exit successful");
  exit(0);
}

int main(int argc, char **argv) {
  int shmid;
  key_t shmkey, qkey;
  int found;
  Message msg;

  /********** INIT **********/
  if ((shmkey = ftok("makefile", 'a')) < 0) {
    EXIT_ON_ERROR
  }
  if ((shmid = shmget(shmkey, sizeof(int), 0644)) < 0) {
    EXIT_ON_ERROR
  }
  if ((executing = shmat(shmid, NULL, 0)) < (int *)0) {
    EXIT_ON_ERROR
  }
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

  srandom(time(NULL));
  /********** END-INIT **********/

  logmsg("Going into execution cycle");
  sleep(1);
  msg.type = getpid();
  while (*executing) {
    while (!found) {
      msg.destination.x = (rand() % SO_WIDTH);
      msg.destination.y = (rand() % SO_HEIGHT);
      if (isFree(mapptr, msg.destination)) {
        found = 1;
      }
    }
    logmsg("Sending message:");
    if(DEBUG){
      printf("\tmsg((%ld),(%d,%d))\n", msg.type,
             msg.destination.x,
             msg.destination.y);
    }

    msgsnd(qid, &msg, sizeof(Point), 0);
    found = 0;
  }
  logmsg("Closing...");
  exit(0);
}
