#include "master.h"

Cell (*mapptr)[][SO_HEIGHT];
volatile int executing = 1;
Data simData;

void cellsData(Cell (*map)[][SO_HEIGHT]) {
  int x, y, n, cnt, tmpIB, tmpIA;
  Point tmpB, tmpT;
  int usage[simData.topCells];
  for (y = 0; y < SO_HEIGHT; y++) {
    for (x = 0; x < SO_HEIGHT; x++) {
        for (n = 0; n < simData.topCells; n++) {
          if ((*map)[x][y].visits > usage[n]) {
            if(n != (simData.topCells-1)){
              tmpT.x = simData.cellsWinner[n].x;
              tmpT.y = simData.cellsWinner[n].y;
              tmpIA = usage[n];
              for (cnt = 0; cnt + n < simData.topCells; cnt++) {
                tmpB.x = simData.cellsWinner[cnt+n+1].x;
                tmpB.y = simData.cellsWinner[cnt+n+1].y;
                tmpIB = usage[cnt+n+1];
             	simData.cellsWinner[cnt+n+1].y = tmpT.y;
                simData.cellsWinner[cnt+n+1].x = tmpT.x;
                usage[cnt+n+1] = tmpIA;
                tmpT.x = tmpB.x; 
                tmpT.y = tmpB.y;
                tmpIA = tmpIB;
              }
            }
            simData.cellsWinner[n].x = x;
            simData.cellsWinner[n].y = y;
            usage[n] = (*map)[x][y].visits;
            break;
          }
        }
    }
  }
}

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
    executing = 0;
    break;
  case SIGALRM:
    executing = 0;
    break;
  case SIGQUIT:
    executing = 0;
    break;
  case SIGUSR1:
    break;
  case SIGUSR2:
    executing = 0;
    break;
  case SIGTSTP:
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

void printReport(Cell (*map)[][SO_HEIGHT]) {
  int x, y, n, db;
  printf("========== Simulation Success ==========\n");
  printf("Statistics:\n");
  printf("\tTrips:\n");
  printf("\t   \tSuccessful \tNot Served \tAborts\n");
  printf("\t   \t%d         \t%d         \t%d\n", simData.tripsSuccess,
         simData.tripsNotServed, (simData.trips - simData.tripsSuccess));
  printf("\tTaxi:\n");
  printf("\t\tMost Distance \tLongest Trip \tMost Trips\n");
  printf("\tpid:\t%ld       \t%ld          \t%ld\n", simData.distanceWinner,
         simData.timeWinner, simData.tripsWinner);
  printf("\t    \t%d            \t%ld ms     \t%d\n", simData.maxDistance,
         (simData.maxTime.tv_sec * 1000 + simData.maxTime.tv_usec / 1000),
         simData.maxTrips);
  printf("\tCells:\n");
  printf("\t\tvisits \tx \ty \tstate\n");
  for(n = 0; n < simData.topCells; n++){
     switch ((*map)[simData.cellsWinner[n].x][simData.cellsWinner[n].y].state) {
      case FREE:
        printf("\t\t%d \t%d \t%d \tFREE\n", (*map)[simData.cellsWinner[n].x][simData.cellsWinner[n].y].visits, simData.cellsWinner[n].x, 
    			simData.cellsWinner[n].y);
        break;
      case SOURCE:
        printf("\t\t%d \t%d \t%d \tSOURCE\n", (*map)[simData.cellsWinner[n].x][simData.cellsWinner[n].y].visits, simData.cellsWinner[n].x, 
    			simData.cellsWinner[n].y);
        break;
      case HOLE:
        printf("[#]");
        break;
    }
  }
  for (y = 0; y < SO_HEIGHT; y++) {
    for (x = 0; x < SO_WIDTH; x++) {
      switch ((*map)[x][y].state) {
      case FREE:
        db = 0;
        for(n = 0; n < simData.topCells; n++)
        	if(simData.cellsWinner[n].x == x && simData.cellsWinner[n].y == y){
        	  db = 1;
        }
       if(db == 0){
          printf("[ ]");
        }else{
          printf( ANSI_COLOR_RED "[*]" ANSI_COLOR_RESET);
        }
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
int main() {
  char *args[2];
  char *envp[1];
  char id_buffer[30];
  int shmid_map, qid, source_qid, t, sem_idM, buffer;
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
  sigaction(SIGQUIT, &act, 0);
  sigaction(SIGUSR1, &act, 0);
  sigaction(SIGUSR2, &act, 0);
  sigaction(SIGTSTP, &act, 0);

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
  msgrcv(source_qid, &msg_source, sizeof(int), 2, 0);
  simData.topCells = msg_source.requests;
  t = time(NULL);
  while (executing) {
    if ((time(NULL) - t) >= 1) {
      printMap(mapptr);
      t = time(NULL);
    }
  }
  while (wait(NULL) > 0) {
  }
  sleep(1);
  msgctl(source_qid, IPC_STAT, &q_ds);
  while (q_ds.msg_qnum > 0) {
    if (msgrcv(source_qid, &msg_source, sizeof(int), 0, IPC_NOWAIT) == -1) {
      perror("msgrcv");
      EXIT_ON_ERROR
    }
    simData.requests += msg_source.requests;
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
  cellsData(mapptr);
  printReport(mapptr);

  if (shmctl(shmid_map, IPC_RMID, NULL)) {
    printf("\nError in shmctl: map,\n");
    EXIT_ON_ERROR
  }
  if (msgctl(source_qid, IPC_RMID, NULL) == -1) {
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
