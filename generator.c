#include "generator.h"
#include "general.h"
#include <signal.h>
#include <unistd.h>
Config conf;
int shmid_sources, shmid_map, shmid_ex, qid, sem_idW, sem_idR, sem_idM;
Point (*sourcesList_ptr)[MAX_SOURCES];
Cell (*mapptr)[][SO_HEIGHT];

int main(int argc, char **argv) {
  int i, xArg, yArg, arg, *executing;
  key_t shmkey, qkey, semkeyW, semkeyR, semkeyM;
  char xArgBuffer[5], yArgBuffer[5], argBuffer1[5], argBuffer2[5],
      argBuffer3[5];
  char *args[7];
  char *envp[1];
  int col, row;
  union semun argR, argW;
  unsigned short semval[SO_WIDTH * SO_HEIGHT];
  int cnt;
  struct sigaction act;
  struct semid_ds idR, idW;

  for (cnt = 0; cnt < SO_WIDTH * SO_HEIGHT; cnt++)
    semval[cnt] = 0;

  /************ INIT ************/

  memset(&act, 0, sizeof(act));
  act.sa_handler = handler;

  sigaction(SIGINT, &act, 0);
  sigaction(SIGALRM, &act, 0);
  sigaction(SIGUSR1, &act, 0);
  sigaction(SIGUSR2, &act, 0);
  sigaction(SIGQUIT, &act, 0);
  if ((shmkey = ftok("./makefile", 'm')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((shmid_map = shmget(shmkey, 0, 0666)) < 0) {
    printf("shmget error\n");
    EXIT_ON_ERROR
  }
  if ((void *)(mapptr = shmat(shmid_map, NULL, 0)) < (void *)0) {
    EXIT_ON_ERROR
  }

  if ((shmkey = ftok("./makefile", 'b')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((shmid_sources = shmget(shmkey, SO_WIDTH * SO_HEIGHT * sizeof(Point),
                              IPC_CREAT | 0644)) < 0) {
    printf("shmget error\n");
    EXIT_ON_ERROR
  }
  if ((void *)(sourcesList_ptr = shmat(shmid_sources, NULL, 0)) < (void *)0) {
    EXIT_ON_ERROR
  }

  if ((qkey = ftok("./makefile", 'q')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((qid = msgget(qkey, IPC_CREAT | 0644)) < 0) {
    printf("msgget error\n");
    EXIT_ON_ERROR
  }

  if ((semkeyR = ftok("./makefile", 'r')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  argR.buf = &idR;
  argR.array = semval;
  if ((sem_idR = semget(semkeyR, SO_WIDTH * SO_HEIGHT, IPC_CREAT | 0666)) < 0) {
    printf("semget error\n");
    EXIT_ON_ERROR
  }
  if (semctl(sem_idR, 0, SETALL, argR) < 0) {
    printf("semctl error\n");
    EXIT_ON_ERROR
  }

  if ((semkeyW = ftok("./makefile", 'w')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((sem_idW = semget(semkeyW, SO_WIDTH * SO_HEIGHT, IPC_CREAT | 0666)) < 0) {
    printf("semget error\n");
    EXIT_ON_ERROR
  }
  argW.buf = &idW;
  argW.array = semval;
  if (semctl(sem_idW, 0, SETALL, argW) < 0) {
    printf("semctl error\n");
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

  parseConf(&conf);
  if (DEBUG) {
    logmsg("Testing Map:", DB);
    for (col = 0; col < SO_WIDTH; col++) {
      for (row = 0; row < SO_HEIGHT; row++) {
        (*mapptr)[col][row].state = FREE;
        (*mapptr)[col][row].capacity = 100;
      }
    }
    logmsg("OK", DB);
  }

  logmsg("Generate map...", DB);
  generateMap(mapptr, &conf);

  logmsg("Init complete", DB);

  srand(time(NULL) + getpid());
  /************ END-INIT ************/

  logmsg("Printing map...", DB);
  printMap(mapptr);
  logmsg("Forking Sources...", DB);

  for (i = 1; i < conf.SO_SOURCES + 1; i++) {
    if (DEBUG) {
      printf("\tCreating Source n. %d\n", i);
    }
    switch (fork()) {
    case -1:
      EXIT_ON_ERROR
    case 0:
      arg = i;
      sprintf(argBuffer1, "%d", arg);
      args[0] = "source";
      args[1] = argBuffer1;
      args[2] = NULL;
      envp[0] = NULL;
      execve( "source", args, envp);
      /* here execv failed */
      EXIT_ON_ERROR
    }
  }

  logmsg("Forking Taxis...", DB);
  for (i = 1; i < conf.SO_TAXI + 1; i++) {
    if (DEBUG) {
      printf("\tTaxi n. %d created\n", i);
    }
    switch (fork()) {
    case -1:
      EXIT_ON_ERROR
    case 0:
      execTaxi();
    }
  }
  if(allInit()){
        EXIT_ON_ERROR
  }
  logmsg("Starting Timer now.", DB);
  alarm(conf.SO_DURATION);
  if (kill(0, SIGUSR1) < 0) {
    EXIT_ON_ERROR
  }
  logmsg("Waiting for Children...", DB);
  while (wait(NULL) > 0) {
  }
  kill(0, SIGINT);
}

/*
 * Parses the file taxicab.conf in the source directory and populates the Config
 * struct
 */

int allInit(){
        struct sembuf init;
        init.sem_num = 0;
        init.sem_op = -1;
        init.sem_flg = 0;
        return semop(sem_idM, &init, 1);
}

void parseConf(Config *conf) {
  FILE *in;
  char s[16], c;
  int n;
  char filename[] = "taxicab.conf";
  in = fopen(filename, "r");
  while (fscanf(in, "%s", s) == 1) {
    switch (s[0]) {
    case '#': /* comment */
      do {
        c = fgetc(in);
      } while (c != '\n');
      break;
    default:
      fscanf(in, "%d\n", &n);
      if (strncmp(s, "SO_TAXI", 7) == 0) {
        conf->SO_TAXI = n;
      } else if (strncmp(s, "SO_SOURCES", 10) == 0) {
        conf->SO_SOURCES = n;
      } else if (strncmp(s, "SO_HOLES", 8) == 0) {
        conf->SO_HOLES = n;
      } else if (strncmp(s, "SO_CAP_MIN", 10) == 0) {
        conf->SO_CAP_MIN = n;
      } else if (strncmp(s, "SO_CAP_MAX", 10) == 0) {
        conf->SO_CAP_MAX = n;
      } else if (strncmp(s, "SO_TIMENSEC_MIN", 15) == 0) {
        conf->SO_TIMENSEC_MIN = n;
      } else if (strncmp(s, "SO_TIMENSEC_MAX", 15) == 0) {
        conf->SO_TIMENSEC_MAX = n;
      } else if (strncmp(s, "SO_TIMEOUT", 10) == 0) {
        conf->SO_TIMEOUT = n;
      } else if (strncmp(s, "SO_DURATION", 11) == 0) {
        conf->SO_DURATION = n;
      }
    }
  }

  fclose(in);
}

/*
 *  Checks for adiacent cells at matrix[x][y] marked as HOLE, returns 0 on
 *  success
 */
int checkNoAdiacentHoles(Cell (*matrix)[][SO_HEIGHT], int x, int y) {
  int b = 0;
  int i, j;
  time_t startTime;

  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      if ((x + i - 1) >= 0 && (x + i - 1) <= SO_WIDTH && (y + j - 1) >= 0 &&
          (y + j - 1) <= SO_HEIGHT &&
          (*matrix)[x + i - 1][y + j - 1].state == HOLE) {
        b = 1;
      }
    }
  }
  return b;
}

/*
 * Populates matrix[][] with Cell struct with status Free, Source, Hole
 */
void generateMap(Cell (*matrix)[][SO_HEIGHT], Config *conf) {
  int x, y, r, i;
  time_t startTime;
  for (x = 0; x < SO_WIDTH; x++) {
    for (y = 0; y < SO_HEIGHT; y++) {
      (*matrix)[x][y].state = FREE;
      (*matrix)[x][y].traffic = 0;
      (*matrix)[x][y].visits = 0;
      r = rand();
      (*matrix)[x][y].capacity =
          (r % (conf->SO_CAP_MAX - conf->SO_CAP_MIN)) + conf->SO_CAP_MIN;
    }
  }
  startTime = time(NULL); /* To stop the user from using too many holes */
  for (i = conf->SO_HOLES; i > 0; i--) {
    if (time(NULL) - startTime > 2) {
      logmsg("I'm sorry, you selected too many holes to fit the map:/n/tRetry "
             "with less. Quitting...",
             RUNTIME);
      exit(0);
    }
    x = rand() % SO_WIDTH;
    y = rand() % SO_HEIGHT;

    if (checkNoAdiacentHoles(matrix, x, y) == 0) {
      (*matrix)[x][y].state = HOLE;
    } else {
      i++;
    }
  }
  startTime = time(NULL); /* To stop the user from using too many sources */
  logmsg("Generating Sources...", DB);
  for (i = 0; i < conf->SO_SOURCES; i++) {
    if (time(NULL) - startTime > 2) {
      logmsg("It seems you selected too many sources to fit the map:/n/tRetry "
             "with less. Quitting...",
             RUNTIME);
      kill(0, SIGINT);
    }
    x = rand() % SO_WIDTH;
    y = rand() % SO_HEIGHT;

    if ((*matrix)[x][y].state == FREE) {
      (*matrix)[x][y].state = SOURCE;
      (*sourcesList_ptr)[i].x = x;
      (*sourcesList_ptr)[i].y = y;
    } else {
      i--;
    }
  }
}

/*
 * Print on stdout the map in a readable format:
 *     FREE Cells are printed as   [ ]
 *     SOURCE Cells are printed as [S]
 *     HOLE Cells are printed as   [#]
 */
void printMap(Cell (*map)[][SO_HEIGHT]) {
  int x, y;
  for (y = 0; y < SO_HEIGHT; y++) {
    for (x = 0; x < SO_WIDTH; x++) {
      switch ((*map)[x][y].state) {
      case FREE:
        printf("[ ]");
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

void logmsg(char *message, enum Level l) {
  if (l <= DEBUG) {
    printf("[generator-%d] %s\n", getpid(), message);
  }
}

void execTaxi() {
  int xArg, yArg;
  char xArgBuffer[5], yArgBuffer[5], argBuffer1[5], argBuffer2[5],
      argBuffer3[5];
  char *args[7];
  char *envp[1];

  srand(time(NULL) ^ (getpid() << 16));
  xArg = (rand() % SO_WIDTH);
  yArg = (rand() % SO_HEIGHT);
  snprintf(xArgBuffer, 5, "%d", xArg);
  snprintf(yArgBuffer, 5, "%d", yArg);
  snprintf(argBuffer1, 5, "%d", conf.SO_TIMENSEC_MIN);
  snprintf(argBuffer2, 5, "%d", conf.SO_TIMENSEC_MAX);
  snprintf(argBuffer3, 5, "%d", conf.SO_TIMEOUT);
  args[0] = "taxi";
  args[1] = xArgBuffer;
  args[2] = yArgBuffer;
  args[3] = argBuffer1;
  args[4] = argBuffer2;
  args[5] = argBuffer3;
  args[6] = NULL;
  envp[0] = NULL;
  execve( "taxi", args, envp);
  EXIT_ON_ERROR
}

void handler(int sig) {
  switch (sig) {
  case SIGINT:
    printf("=============== Received SIGINT ==============\n");
    shmdt(mapptr);
    shmdt(sourcesList_ptr);
    if (shmctl(shmid_sources, IPC_RMID, NULL)) {
      printf("\nError in shmctl: sources,\n");
      EXIT_ON_ERROR
    }
    if (semctl(sem_idW, 0, IPC_RMID)) {
      printf("\nError in shmctl: sources,\n");
      EXIT_ON_ERROR
    }
    if (semctl(sem_idR, 0, IPC_RMID)) {
      printf("\nError in shmctl: sources,\n");
      EXIT_ON_ERROR
    }
    if (msgctl(qid, IPC_RMID, NULL)) {
      printf("\nError in msgctl,\n");
      EXIT_ON_ERROR
    }
    logmsg("Graceful exit successful", DB);
    if (kill(0, SIGUSR2) < 0) {
      EXIT_ON_ERROR
    }
    exit(0);
    break;
  case SIGALRM:
    if (kill(0, SIGINT) < 0) {
      EXIT_ON_ERROR
    }
    break;
  case SIGQUIT:
    switch(fork()){
    case -1:
      EXIT_ON_ERROR
    case 0:
      execTaxi();
    }
  case SIGUSR1:
    break;
  case SIGUSR2:
    break;
  }
}
