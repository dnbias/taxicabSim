#include "source.h"
MasterMessage msg_master;
Cell (*mapptr)[][SO_HEIGHT];
int qid, master_qid, sem_idW, sem_idR;

int main(int argc, char **argv) {

  int shmid, sem_idR, sem_idW, sem_idM;
  key_t shmkey, qkey, semkeyW, semkeyR, semkeyM;

  int found = 0;
  Message msg;
  struct timespec msgInterval;
  struct sigaction act;

  /********** INIT **********/
  memset(&act, 0, sizeof(act));
  act.sa_handler = handler;

  sigaction(SIGINT, &act, 0);
  sigaction(SIGALRM, &act, 0);
  sigaction(SIGUSR1, &act, 0);
  sigaction(SIGUSR2, &act, 0);

  if ((shmkey = ftok("./makefile", 'm')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((shmid = shmget(shmkey, 0, 0644)) < 0) {
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
  if ((qid = msgget(qkey, IPC_CREAT | 0644)) < 0) {
    EXIT_ON_ERROR
  }

   /*  queue for comunication with master */
  if ((qkey = ftok("./makefile", 's')) < 0) {
    EXIT_ON_ERROR
  }
  if ((master_qid = msgget(qkey, 0644)) < 0) {
    EXIT_ON_ERROR
  }

  if ((semkeyR = ftok("./makefile", 'r')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((sem_idR = semget(semkeyR, 0, 0)) < 0) {
    printf("semget error\n");
    EXIT_ON_ERROR
  }
  if ((semkeyW = ftok("./makefile", 'w')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((sem_idW = semget(semkeyW, 0, 0)) < 0) {
    printf("semget error\n");
    EXIT_ON_ERROR
   }

  if ((semkeyR = ftok("./makefile", 'r')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((sem_idR = semget(semkeyR, 0, 0)) < 0) {
    printf("semget error\n");
    EXIT_ON_ERROR
  }
  if ((semkeyW = ftok("./makefile", 'w')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((sem_idW = semget(semkeyW, 0, 0)) < 0) {
    printf("semget error\n");
    EXIT_ON_ERROR
  }


  if ((semkeyM = ftok("./makefile", 'm')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((sem_idM = semget(semkeyM, 0, 0)) < 0) {
    printf("semget error\n");
    EXIT_ON_ERROR
  }
  srand(time(NULL) ^ (getpid() << 16));
  sscanf(argv[1], "%ld", &msg.type);
  msgInterval.tv_sec = 0;
  msgInterval.tv_nsec = 200000000;
  msg_master.type = 1;
  msg_master.requests = 0;
  /********** END-INIT **********/
  if(isInit(sem_idM)){
    EXIT_ON_ERROR
  }
  logmsg("Going into execution cycle", DB);
  if (DEBUG)
    while (1) {
      nanosleep(&msgInterval, NULL);
      while (found != 1) {
        msg.destination.x = (rand() % SO_WIDTH);
        msg.destination.y = (rand() % SO_HEIGHT);
        if (msg.destination.x > 0 && msg.destination.x < SO_WIDTH &&
            msg.destination.y > 0 && msg.destination.y < SO_HEIGHT) {
          if (isFree(mapptr, msg.destination, sem_idR, sem_idW)) {
            found = 1;
          }
        }
      }
      logmsg("Sending message:", DB);
      if (DEBUG) {
        printf("\tmsg((%ld),(%d,%d))\n", msg.type, msg.destination.x,
               msg.destination.y);
      }
      msgsnd(qid, &msg, sizeof(Point), 0);
      msg_master.requests++;
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

void handler(int sig) {
  switch (sig) {
  case SIGINT:
    logmsg("Finishing up", DB);
    shmdt(mapptr);
    msgsnd(master_qid, &msg_master, sizeof(int), 0);
    logmsg("Graceful exit successful", DB);
    exit(0);
  case SIGUSR1:
    break;
  }
}
