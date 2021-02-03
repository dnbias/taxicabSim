#include "source.h"
MasterMessage msg_master;
Cell (*mapptr)[][SO_HEIGHT];
int master_qid, *readers;
int shmid, qid, found = 0, sem, writers, mutex;

int main(int argc, char **argv) {
  /*int shmid, qid, found = 0, sem, writers, mutex;*/
  key_t key;
  Message msg;
  struct timespec msgInterval;
  struct sigaction act;

  /********** INIT **********/
  logmsg("Init", DB);

  memset(&act, 0, sizeof(act));
  act.sa_handler = handler;

  sigaction(SIGINT, &act, 0);
  sigaction(SIGALRM, &act, 0);
  sigaction(SIGUSR1, &act, 0);
  sigaction(SIGUSR2, &act, 0);
  sigaction(SIGTSTP, &act, 0);

  if ((key = ftok("./makefile", 'm')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((shmid = shmget(key, 0, 0644)) < 0) {
    printf("shmget error\n");
    EXIT_ON_ERROR
  }

  if ((void *)(mapptr = shmat(shmid, NULL, 0)) < (void *)0) {
    logmsg("ERROR shmat - mapptr", RUNTIME);
    EXIT_ON_ERROR
  }

  if ((key = ftok("./makefile", 'z')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((shmid = shmget(key, 0, 0666)) < 0) {
    EXIT_ON_ERROR
  }
  if ((void *)(readers = shmat(shmid, NULL, 0)) < (void *)0) {
    logmsg("ERROR shmat - readers", RUNTIME);
    EXIT_ON_ERROR
  }

  /* queue for comunication with taxis */
  if ((key = ftok("./makefile", 'q')) < 0) {
    EXIT_ON_ERROR
  }
  if ((qid = msgget(key, IPC_CREAT | 0644)) < 0) {
    EXIT_ON_ERROR
  }
  /*  queue for comunication with master */
  if ((key = ftok("./makefile", 's')) < 0) {
    EXIT_ON_ERROR
  }
  if ((master_qid = msgget(key, 0644)) < 0) {
    EXIT_ON_ERROR
  }

  /* Semaphores */
  if ((key = ftok("./makefile", 'y')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((sem = semget(key, 0, 0666)) < 0) {
    printf("semget error\n");
    EXIT_ON_ERROR
  }
  if ((key = ftok("./makefile", 'w')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((writers = semget(key, 0, 0666)) < 0) {
    printf("semget error\n");
    EXIT_ON_ERROR
  }
  if ((key = ftok("./makefile", 'm')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((mutex = semget(key, 0, 0666)) < 0) {
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

  semSyncSource(sem);

  logmsg("Going into execution cycle", DB);
  while (1) {
    nanosleep(&msgInterval, NULL);
    while (found != 1) {
      msg.destination.x = (rand() % SO_WIDTH);
      msg.destination.y = (rand() % SO_HEIGHT);
      if (msg.destination.x >= 0 && msg.destination.x < SO_WIDTH &&
          msg.destination.y >= 0 && msg.destination.y < SO_HEIGHT) {
        semWait(msg.destination, mutex);
        *readers++;
        if (*readers == 1)
          semWait(msg.destination, writers);
        semSignal(msg.destination, mutex);
        found = isFree(mapptr, msg.destination);
        semWait(msg.destination, mutex);
        *readers--;
        if (*readers == 0)
          semSignal(msg.destination, writers);
        semSignal(msg.destination, mutex);
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

void unblock(int sem) {
  struct sembuf buf;
  buf.sem_num = 0;
  buf.sem_op = -1;
  buf.sem_flg = 0;
  if (semop(sem, &buf, 1) < 0)
    EXIT_ON_ERROR
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
    shmdt(readers);
    msgsnd(master_qid, &msg_master, sizeof(int), 0);
    logmsg("Graceful exit successful", DB);
    exit(0);
  case SIGUSR1:
    logmsg("Received SIGUSR1", DB);
    break;
  case SIGTSTP:
    logmsg("Received SIGSTOP", RUNTIME);
    utentRequest();
    break;
    return;
  }
}
int semSyncSource(int sem) {
  struct sembuf buf;
  buf.sem_num = 0;
  buf.sem_op = 0;
  buf.sem_flg = 0;
  if (semop(sem, &buf, 1) < 0)
    return -1;
  else
    return 0;
}
void utentRequest() {
  Message msg;
  int found;
  if (semSyncSource(sem) < 0)
    kill(getppid(), SIGUSR2);
  found = 1;
  while (found != 0) {
    msg.destination.x = (rand() % SO_WIDTH);
    msg.destination.y = (rand() % SO_HEIGHT);
    if (msg.destination.x >= 0 && msg.destination.x < SO_WIDTH &&
        msg.destination.y >= 0 && msg.destination.y < SO_HEIGHT) {
      semWait(msg.destination, mutex);
      *readers++;
      if (*readers == 1)
        semWait(msg.destination, writers);
      semSignal(msg.destination, mutex);
      found = isFree(mapptr, msg.destination);
      semWait(msg.destination, mutex);
      *readers--;
      if (*readers == 0)
        semSignal(msg.destination, writers);
      semSignal(msg.destination, mutex);
    }
    found = 0;
  }
  logmsg(ANSI_COLOR_RED "Sending message:" ANSI_COLOR_RESET, DB);
  if (DEBUG) {
    printf("\tmsg((%ld),(%d,%d))\n", msg.type, msg.destination.x,
           msg.destination.y);
  }
  msgsnd(qid, &msg, sizeof(Point), 0);
  msg_master.requests++;
  return;
}
