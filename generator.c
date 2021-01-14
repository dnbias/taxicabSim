#include "generator.h"
#include "general.h"

int shmid_sources, shmid_map, shmid_ex, qid;
Point (*sources_ptr)[MAX_SOURCES];
Cell (*mapptr)[][SO_HEIGHT];

int main(int argc, char **argv) {
  Config conf;
  int i, xArg, yArg, *executing;
  key_t shmkey, qkey;
  char xArgBuffer[20], yArgBuffer[20];
  char *args[4];
  char *envp[1];

  int col, row;
  /************ INIT ************/
  logmsg("Initialization", DB);

  if ((shmkey = ftok("./.gitignore", 'm')) < 0) {
    EXIT_ON_ERROR
  }
  if ((shmid_map = shmget(shmkey, 0, 0644)) < 0) {
    EXIT_ON_ERROR
  }
  if ((void *)(mapptr = shmat(shmid_map, NULL, 0)) < (void *)0) {
    EXIT_ON_ERROR
  }

  if ((shmkey = ftok("./.gitignore", 's')) < 0) {
    EXIT_ON_ERROR
  }
  if ((shmid_sources =
           shmget(shmkey, MAX_SOURCES * sizeof(Point), IPC_CREAT | 0666)) < 0) {
    EXIT_ON_ERROR
  }
  if ((void *)(sources_ptr = shmat(shmid_sources, NULL, 0)) < (void *)0) {
    EXIT_ON_ERROR
  }

  if ((qkey = ftok("./.gitignore", 'q')) < 0) {
    EXIT_ON_ERROR
  }
  if ((qid = msgget(qkey, IPC_CREAT | 0644)) < 0) {
    EXIT_ON_ERROR
  }
  parseConf(&conf);
  logmsg("Testing Map:", DB);
  for (col = 0; col < SO_WIDTH; col++) {
    for (row = 0; row < SO_HEIGHT; row++) {
      (*mapptr)[col][row].state = FREE;
      (*mapptr)[col][row].capacity = 100;
    }
  }
  logmsg("OK", DB);

  logmsg("Generate map...", DB);
  generateMap(mapptr, &conf);
  signal(SIGINT, SIGINThandler);
  signal(SIGALRM, ALARMhandler);
  logmsg("Init complete", DB);
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
      args[0] = "source";
      args[1] = NULL;
      envp[0] = NULL;
      execve("source", args, envp);
      /* here execv failed */
      EXIT_ON_ERROR
    }
  }

  logmsg("Forking Taxis...", DB);
  for (i = 0; i < conf.SO_TAXI; i++) {
    if (DEBUG) {
      printf("\tTaxi n. %d created\n", i);
      sleep(1);
    }
    switch (fork()) {
    case -1:
      EXIT_ON_ERROR
    case 0:
      xArg = (rand() % SO_WIDTH);
      yArg = (rand() % SO_HEIGHT);
      snprintf(xArgBuffer, 20, "%d", xArg);
      snprintf(yArgBuffer, 20, "%d", yArg);
      args[0] = "taxi";
      args[1] = xArgBuffer;
      args[2] = yArgBuffer;
      args[3] = NULL;
      envp[0] = NULL;
      execve("taxi", args, envp);
      /* here execve failed */
      EXIT_ON_ERROR
    }
  }
  logmsg("Starting Timer now", DB);
  alarm(conf.SO_DURATION);
  logmsg("Waiting for Children.", DB);
  while (wait(NULL) > 0) {
  }
  exit(0);
}

/*
 * Parses the file taxicab.conf in the source directory and populates the Config
 * struct
 */
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
  srand(time(NULL));
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

  for (i = conf->SO_HOLES; i > 0; i--) {
    x = rand() % SO_WIDTH;
    y = rand() % SO_HEIGHT;

    if (checkNoAdiacentHoles(matrix, x, y) == 0) {
      (*matrix)[x][y].state = HOLE;
    } else {
      i++;
    }
  }
  logmsg("Generating Sources...", DB);
  for (i = 0; i < conf->SO_SOURCES; i++) {
    x = rand() % SO_WIDTH;
    y = rand() % SO_HEIGHT;

    if ((*matrix)[x][y].state == FREE) {
      (*matrix)[x][y].state = SOURCE;
      (*sources_ptr)[i].x = x;
      (*sources_ptr)[i].y = y;
    } else {
      i--;
    }
  }
}

/*
 * Print on stdout the map in a readable format:
 *     FREE Cells are printed as   [ ]
 *     SOURCE Cells are printed as [S]
 *     HOLE Cells are printed as   [X]
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
        printf("[X]");
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

/*  Signal Handlers  */
void SIGINThandler(int sig) {
  printf("=============== Received SIGINT ==============\n");
  while (wait(NULL) > 0) {
  }
  shmdt(mapptr);
  shmdt(sources_ptr);
  if (shmctl(shmid_sources, IPC_RMID, NULL)) {
    printf("\nError in shmctl: sources,\n");
    EXIT_ON_ERROR
  }
  if (msgctl(qid, IPC_RMID, NULL)) {
    printf("\nError in msgctl,\n");
    EXIT_ON_ERROR
  }
  logmsg("Graceful exit successful", DB);
  exit(0);
}

void ALARMhandler(int sig) {
  signal(SIGINT, SIGINThandler);
  printf("=============== Received ALARM ===============\n");
  kill(0, SIGINT);
}
