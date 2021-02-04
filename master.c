#include "master.h"

Cell (*mapptr)[][SO_HEIGHT];
Cell (*mostUsed)[5];
volatile int executing = 1;
Data simData;

/*void cellsData(Cell (*map)[][SO_HEIGHT]){
  int x,y,n;
  (*mostUsed)[0].visits = 0;
  (*mostUsed)[1].visits = 0;
  (*mostUsed)[2].visits = 0;
  (*mostUsed)[3].visits = 0;
  (*mostUsed)[4].visits = 0;
  for(x = 0; x < SO_WIDTH; x++)
	for(y = 0; y < SO_HEIGHT; y++)
	  for(n = 0; n < 4; n++)
		if((*map)[x][y].visits > (*mostUsed)[n].visits){
		  (*mostUsed)[n+1] = (*mostUsed)[n];
		  (*mostUsed)[n] = (*mapptr)[x][y]; 
		}

}*/

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

void printReport() {
  printf("========== Simulation Success ==========\n");
  printf("Statistics:\n");
  printf("\tTrips:\n");
  printf("\t   \tSuccessful \tNot Served \tAborts\n");
  printf("\t   \t%d         \t%d         \t%d\n", simData.tripsSuccess,
         simData.tripsNotServed, (simData.trips - simData.tripsSuccess));
  printf("\tTaxi:\n");
  printf("\t\tMost Distance \tLongest Trip \tMost Trips\n");
  printf("\tpid:\t%ld        \t%ld          \t%ld\n", simData.distanceWinner,
         simData.timeWinner, simData.tripsWinner);
  printf("\t    \t%d            \t%ld ms     \t%d\n", simData.maxDistance,
         (simData.maxTime.tv_sec * 1000 + simData.maxTime.tv_usec / 1000),
         simData.maxTrips);

/*  for (y = 0; y < SO_HEIGHT; y++) {
    for (x = 0; x < SO_WIDTH; x++) {
      switch ((*mapptr)[x][y].state) {
      case FREE:
        for(n = 0; n < 5; n++)
          if((*mapptr)[x][y].visits == (*mostUsed)[n].visits)
          	printf(ANSI_COLOR_RED "[%d]" ANSI_COLOR_RESET, (*mapptr)[x][y].visits);
          else
          	printf("[%d]", (*mapptr)[x][y].visits);
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
  printf("\n");*/
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
  /*cellsData(mapptr);*/
  printReport();

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
