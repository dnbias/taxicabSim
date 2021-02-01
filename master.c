#include "master.h"

Cell (*mapptr)[][SO_HEIGHT];
volatile int executing = 1;
Data simData;

void printMap(Cell (*map)[][SO_HEIGHT]) {
  int x, y;
  for (y = 0; y < SO_HEIGHT; y++) {
    for (x = 0; x < SO_WIDTH; x++) {
      switch ((*map)[x][y].state) {
      case FREE:
        printf("[%d]", (*map)[x][y].traffic);
        break;
      case SOURCE:
        printf("[S]");
        break;
      case HOLE:
        printf("[#]");
      }
    }
    printf("\n");
  }
  printf("\n");
}
void handler(int sig) {
  switch (sig) {
  case SIGINT:
    break;
  case SIGALRM:
    printMap(mapptr);
    alarm(1);
    break;
  case SIGUSR1:
    break;
  case SIGUSR2:
    executing = 0;
    break;
  }
}

void logmsg(char *message, enum Level l) {
  if (l <= DEBUG) {
    printf("[master-%d] %s\n", getpid(), message);
  }
}
void updateData(long pid, taxiData *data) {
  simData.trips = simData.trips + (*data).clients;
  simData.tripsSuccess = simData.tripsSuccess + (*data).tripsSuccess;

  if (simData.maxTrips < (*data).clients) {
    simData.maxTrips = (*data).clients;
    simData.tripsWinner = pid;
  }
  if (simData.maxTime.tv_sec <= (*data).maxTimeInTrip.tv_sec) {
    if (simData.maxTime.tv_usec < (*data).maxTimeInTrip.tv_usec) {
      simData.maxTime.tv_sec = (*data).maxTimeInTrip.tv_sec;
      simData.maxTime.tv_usec = (*data).maxTimeInTrip.tv_usec;
      simData.timeWinner = pid;
    }
  }
  if (simData.maxDistance < (*data).distance) {
    simData.maxDistance = (*data).distance;
    simData.distanceWinner = pid;
  }
}

void printReport() {
  printf("======== Simulazione conclusa ========\n");
  printf("Statistiche:\n");
  printf("\tViaggi:\n");
  printf("\t\tEseguiti con successo\tInevasi\tAbortiti\n");
  printf("\t\t%d                   \t%d     \t%d\n", simData.tripsSuccess,
         simData.tripsNotServed, (simData.trips - simData.tripsSuccess));
  printf("\tTaxi:\n");
  printf("\t\tMaggior Strada\tViaggio piu' lungo\tMaggior numero di viaggi\n");
  printf("\t\t%ld           \t%ld               \t%ld\n",
         simData.distanceWinner, simData.timeWinner, simData.tripsWinner);
  printf("\t\t%d            \t%ld               \t%d\n", simData.maxDistance,
         simData.maxTime.tv_usec, simData.maxTrips);
}

int main() {
  char *args[2];
  char *envp[1];
  char id_buffer[30];
  int shmid_map, qid, source_qid, t, sem_idM , buffer;
  key_t shmkey, qkey, semkeyM;
  dataMessage msg;
  sourceMessage msg_source;
  /*taxiData dataBuffer;*/
  struct msqid_ds q_ds;
  struct sigaction act;
  union semun argM;

  memset(&act, 0, sizeof(act));
  act.sa_handler = handler;

  sigaction(SIGINT, &act, 0);
  sigaction(SIGALRM, &act, 0);
  sigaction(SIGUSR1, &act, 0);
  sigaction(SIGUSR2, &act, 0);

  if ((shmkey = ftok("./makefile", 'm')) < 0) {
    EXIT_ON_ERROR
  }
  if ((shmid_map = shmget(shmkey, SO_WIDTH * SO_HEIGHT * sizeof(Cell),
                          IPC_CREAT | 0666)) < 0) {
    EXIT_ON_ERROR
  }
  if ((void *)(mapptr = shmat(shmid_map, NULL, 0)) < (void *)0) {
    EXIT_ON_ERROR
  }
  /*  queues for comunication with other modules */
  if ((qkey = ftok("./makefile", 's')) < 0) {
    EXIT_ON_ERROR
  }
  if ((source_qid = msgget(qkey, IPC_CREAT | 0644)) < 0) {
    EXIT_ON_ERROR
  }

  if ((qkey = ftok("./makefile", 'd')) < 0) {
    EXIT_ON_ERROR
  }
  if ((qid = msgget(qkey, IPC_CREAT | 0644)) < 0) {
    EXIT_ON_ERROR
  }

  if (DEBUG) {
    logmsg("Testing Map", DB);
    (*mapptr)[10][1].state = FREE;
    (*mapptr)[2][7].state = FREE;
    (*mapptr)[4][1].state = FREE;
    (*mapptr)[1][1].capacity = 70;
    (*mapptr)[14][5].state = FREE;
    (*mapptr)[4][1].capacity = 50;
    (*mapptr)[3][9].state = FREE;
    logmsg("Ok", DB);
  }

  logmsg("Launching Generator", DB);
  switch (fork()) {
  case -1:
    EXIT_ON_ERROR
  case 0:
    args[0] = "generator";
    args[1] = NULL;
    envp[0] = NULL;
    execve("generator", args, envp);
  }

  pause();
  t = time(NULL);
  while (executing) {
    if ((time(NULL) - t) >= 1) {
      printMap(mapptr);
      t = time(NULL);
    }
  }
  while (wait(NULL) > 0) {
  }
  msgctl(source_qid, IPC_STAT, &q_ds);
  while (q_ds.msg_qnum > 0) {
    if (msgrcv(source_qid, &buffer, sizeof(int), 0, IPC_NOWAIT) == -1) {
      perror("msgrcv");
      EXIT_ON_ERROR
    }
    simData.requests += buffer;
    msgctl(source_qid, IPC_STAT, &q_ds);
  }
  msgctl(qid, IPC_STAT, &q_ds);
  while (q_ds.msg_qnum > 0) {
    if (msgrcv(qid, &msg, sizeof(taxiData), 0, IPC_NOWAIT) == -1) {
      perror("msgrcv");
      EXIT_ON_ERROR
    }
    updateData(msg.type, &msg.data);
    msgctl(qid, IPC_STAT, &q_ds);
  }
  simData.tripsNotServed = simData.requests - simData.trips;
  printReport();

  if (shmctl(shmid_map, IPC_RMID, NULL)) {
    printf("\nError in shmctl: map,\n");
    EXIT_ON_ERROR
  }
  if (msgctl(qid, IPC_RMID, NULL) == -1) {
    printf("\nError in shmctl: map,\n");
    EXIT_ON_ERROR
  }

  logmsg("Quitting", DB);
  exit(0);
}
