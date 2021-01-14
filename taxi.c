#include "taxi.h"
Cell (*mapptr)[][SO_HEIGHT];
Point (*sources_ptr)[MAX_SOURCES];
Point position;
int qid;

int main(int argc, char **argv) {
  int shmid;
  key_t shmkey, qkey;
  Message msg;

  logmsg("Init...", DB);
  if (DEBUG)
    sleep(1);

  /************INIT************/
  if ((shmkey = ftok("./.gitignore", 's')) < 0) {
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
  if ((void *)(sources_ptr = shmat(shmid, NULL, 0)) < (void *)0) {
    EXIT_ON_ERROR
  }

  if ((shmkey = ftok("./.gitignore", 'm')) < 0) {
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
  if ((void *)(mapptr = shmat(shmid, NULL, 0)) < (void *)0) {
    logmsg("ERROR shmat - mapptr", RUNTIME);
    EXIT_ON_ERROR
  }
  if ((qkey = ftok("./.gitignore", 'q')) < 0) {
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

  while (1) {
    msgrcv(qid, &msg, sizeof(Point), 0, 0);
    logmsg("Going to Nearest Source", DB);
    /* moveTo(getNearSource()); ********** TODO *********/
    moveTo(getNearSource());
    logmsg("Going to destination", DB);
    moveTo(msg.destination);
  }
}

void moveTo(Point p) { /*pathfinding*/
  logmsg("Moving to", DB);
  incTrafficAt(p);
}

void incTrafficAt(Point p) {
  /*wait mutex*/
  (*mapptr)[p.x][p.y].traffic++;
  logmsg("Incrementato traffico in", DB);
  if (DEBUG)
    printf("\t(%d,%d)\n", p.x, p.y);
  /*signal mutex*/
}

void logmsg(char *message, enum Level l) {
  if (l <= DEBUG) {
    printf("[taxi-%d] %s\n", getpid(), message);
  }
}

void SIGINThandler(int sig) {
  logmsg("Finishing up", DB);
  shmdt(mapptr);
  shmdt(sources_ptr);
  logmsg("Graceful exit successful", DB);
  exit(0);
}

Point getNearSource() {
  Point s;
  int n, temp, d = INT_MAX;
  for (n = 0; n < MAX_SOURCES; n++) {
    temp = abs(position.x - (*sources_ptr)[n].x) +
           abs(position.y - (*sources_ptr)[n].y);
    if (d > temp)
      d = temp;
    s = (*sources_ptr)[n];
  }
  return s;
}
