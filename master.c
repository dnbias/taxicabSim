#include "general.h"
#include "generator.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
Cell (*mapptr)[][SO_HEIGHT];
int executing = 1;
typedef struct {
  int distance;
  int clients;
  int tripsSuccess;
  long maxTimeForTrip;
} taxiData;
typedef struct {
  long type;
  taxiData data;
} dataMessage;

typedef struct {
  int trips;
  int tripsSuccess;
  int tripsNotServed;
  int maxTrips;
  long tripsWinner;
  long maxTime;
  long timeWinner;
  int maxDistance;
  long distanceWinner;
} Data;
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
}

void ALARMhandler(int sig) {
  printMap(mapptr);
  alarm(1);
}
void SIGUSR1handler(int sig) {}

void SIGUSR2handler(int sig) { executing = 0; }

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
  if (simData.maxTime < (*data).maxTimeForTrip) {
    simData.maxTime = (*data).maxTimeForTrip;
    simData.timeWinner = pid;
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
         simData.maxTime, simData.maxTrips);
}

int main() {
  char *args[2];
  char *envp[1];
  char id_buffer[30];
  int shmid_map, qid;
  key_t shmkey, qkey;
  dataMessage msg;
  taxiData dataBuffer;
  struct msqid_ds q_ds;

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
  /*  queue for comunication with other modules */
  if ((qkey = ftok("./makefile", 'd')) < 0) {
    EXIT_ON_ERROR
  }
  if ((qid = msgget(qkey, IPC_CREAT | 0644)) < 0) {
    EXIT_ON_ERROR
  }
  signal(SIGALRM, ALARMhandler);
  signal(SIGUSR1, SIGUSR1handler);
  signal(SIGUSR2, SIGUSR2handler);

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
  while (executing) {
  }
  while (wait(NULL) > 0) {
  }
  msgctl(qid, IPC_STAT, &q_ds);
  while (q_ds.msg_qnum > 0) {
    if (msgrcv(qid, &dataBuffer, sizeof(msg.data), 0, IPC_NOWAIT) == -1) {
      perror("msgrcv");
      EXIT_ON_ERROR
    }
    updateData(msg.type, &dataBuffer);
    msgctl(qid, IPC_STAT, &q_ds);
  }
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
