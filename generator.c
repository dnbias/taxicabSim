#include "generator.h"
#include "general.h"
#include <sys/ipc.h>
#include <sys/shm.h>

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
int checkNoAdiacentHoles(Cell (*matrix)[SO_WIDTH][SO_HEIGHT], int x, int y) {
  int b = 0;
  int i, j;
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      if ((x + i - 1) >= 0 && (x + i - 1) <= SO_WIDTH && (y + j - 1) >= 0 &&
          (y + j - 1) <= SO_HEIGHT && !(i == 1 && j == 1)) {
        b = b && (matrix[x + i - 1][y + j - 1]->state != HOLE);
      }
    }
  }
  return b;
}

/*
 * Populates matrix[][] with Cell struct with status Free, Source, Hole
 */
void generateMap(Cell (*matrix)[SO_WIDTH][SO_HEIGHT], Config *conf) {
  int x, y, r, i;
  srandom(time(NULL));
  for (x = 0; x < SO_WIDTH; x++) {
    for (y = 0; y < SO_HEIGHT; y++) {
      matrix[x][y]->state = FREE;
      matrix[x][y]->traffic = 0;
      matrix[x][y]->visits = 0;
      r = rand();
      matrix[x][y]->capacity =
          (r % (conf->SO_CAP_MAX - conf->SO_CAP_MIN)) + conf->SO_CAP_MIN;
    }
  }

  for (i = conf->SO_HOLES; i > 0; i--) {
    x = rand() % SO_WIDTH;
    y = rand() % SO_HEIGHT;

    if (checkNoAdiacentHoles(matrix, x, y) == 0) {
      matrix[x][y]->state = HOLE;
    } else {
      i++;
    }
  }
  for (i = conf->SO_SOURCES; i > 0; i--) {
    x = rand() % SO_WIDTH;
    y = rand() % SO_HEIGHT;

    if (matrix[x][y]->state != HOLE && matrix[x][y]->state != SOURCE) {
      i++;
    } else {
      matrix[x][y]->state = SOURCE;
    }
  }
}

/*
 * Detaches and eliminates shared memory segment
 */
void cleanup(const void *adr, int shmid) {
  shmdt(adr);
  shmctl(shmid, IPC_RMID, 0);
}

int main(int argc, char **argv) {
  /*Cell map[SO_WIDTH][SO_HEIGHT];*/
  Config conf;
  int i, shmid;
  key_t shmkey;
  void *mapptr;
  /* Genero chiave unica per tutti i processi */
  if ((shmkey = ftok("makefile", 'd')) < 0) {
    EXIT_ON_ERROR
  }
  if ((shmid = shmget(shmkey, SO_WIDTH * SO_HEIGHT * sizeof(Cell),
                      IPC_CREAT | 0644)) < 0) {
    EXIT_ON_ERROR
  }
  /* mapptr = &map;*/
  if ((mapptr = shmat(shmid, NULL, 0)) < (void *)0) {
    EXIT_ON_ERROR
  }
  parseConf(&conf);
  generateMap(mapptr, &conf);
  printMap(mapptr);

  for (i = 0; i < conf.SO_TAXI; i++) {
    switch (fork()) {
    case -1:
      EXIT_ON_ERROR
    case 0:
      execl("taxi", "taxi", (char *)NULL);
      /* here execv failed */
      EXIT_ON_ERROR
    }
  }
  /* Here the parent should manage the requests */
  /*
  while (0) {
  }
  */

  cleanup(mapptr, shmid);
  return 0;
}
