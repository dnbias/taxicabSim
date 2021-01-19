#include "source.h"

Cell (*mapptr)[][SO_HEIGHT];
int qid;

int main(int argc, char **argv) {
  int shmid;
  key_t shmkey, qkey;
  int found;
  Message msg;
  struct timespec msgInterval;
  /********** INIT **********/
  if ((shmkey = ftok("./makefile", 'm')) < 0) {
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
  if ((qkey = ftok("./makefile", 'q')) < 0) {
    EXIT_ON_ERROR
  }
  if ((qid = msgget(qkey, 0644)) < 0) {
    EXIT_ON_ERROR
  }
  signal(SIGINT, SIGINThandler); /* TODO implementare con sigaction() */
  srand(time(NULL) + getpid());
  sscanf(argv[1], "%d", &msg.type);

  msgInterval.tv_sec = 0;
  msgInterval.tv_nsec = 200000000;
  /********** END-INIT **********/

  logmsg("Going into execution cycle", DB);
  if (DEBUG)
    while (1) {
      nanosleep(&msgInterval, NULL);
      while (!found) {
        msg.destination.x = (rand() % SO_WIDTH);
        msg.destination.y = (rand() % SO_HEIGHT);
        if (isFree(mapptr, msg.destination)) {
          found = 1;
        }
      }
      logmsg("Sending message:", DB);
      if (DEBUG) {
        printf("\tmsg((%d),(%d,%d))\n", msg.type, msg.destination.x,
               msg.destination.y);
      }
      msgsnd(qid, &msg, sizeof(Point), 0);
      found = 0;
    }
}

void logmsg(char *message, enum Level l) {
  int pid;
  if (l <= DEBUG) {
    pid = getpid();
    printf("[source-%d] %s\n", pid, message);
  }
}

void SIGINThandler(int sig) { /* TODO implement signal-safety */
  logmsg("Finishing up", DB);
  shmdt(mapptr);
  logmsg("Graceful exit successful", DB);
  exit(0);
}
