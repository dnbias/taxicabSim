#include "taxi.h"
Cell (*mapptr)[][SO_HEIGHT];
Point (*sourcesList_ptr)[MAX_SOURCES];
Point position;
int qid, timensec_min, timensec_max, timeout;

int main(int argc, char **argv) {
  int shmid, source_id;
  key_t shmkey, qkey;
  Message msg;
  const struct timespec nsecs[] = {0, 500000000L};
  logmsg("Init...", DB);

  /************INIT************/
  if ((shmkey = ftok("./.gitignore", 's')) < 0) {
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
  if ((void *)(sourcesList_ptr = shmat(shmid, NULL, 0)) < (void *)0) {
    EXIT_ON_ERROR
  }

  if ((shmkey = ftok("./.gitignore", 'm')) < 0) {
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
  if ((qkey = ftok("./.gitignore", 'q')) < 0) {
    EXIT_ON_ERROR
  }
  if ((qid = msgget(qkey, 0644)) < 0) {
    EXIT_ON_ERROR
  }

  signal(SIGINT, SIGINThandler);
  sscanf(argv[1], "%d", &position.x);
  sscanf(argv[2], "%d", &position.y);
  sscanf(argv[3], "%d", &timensec_min);
  sscanf(argv[4], "%d", &timensec_max);
  sscanf(argv[5], "%d", &timeout);
  logmsg("Init Finished", DB);
  srand(time(NULL));
  /************END-INIT************/

  incTrafficAt(position);

  while (1) {
    logmsg("Going to Nearest Source", DB);
    moveTo(getNearSource(&source_id));
    msgrcv(qid, &msg, sizeof(Point), source_id, 0);
    logmsg("Going to destination", DB);
    moveTo(msg.destination);
  }
}

void moveTo(Point dest) { /*pathfinding*/
  int dirX, dirY, i, found;
  long t1, t2;
  struct timespec transit;
  Point temp;
  logmsg("Moving to", DB);
  while (position.x != dest.x && position.y != dest.y) {
    dirX = dest.x - position.x;
    dirY = dest.y - position.y;
    temp.x = position.x;
    temp.y = position.y;
    found = 0;
    if (dirX > 0 && dirY > 0) {
      temp.x++;
      for (i = 0; i < 3 && !found; i++) {
        switch (isFree(mapptr, temp)) {
        case 1:
          incTrafficAt(temp);
          position.x = temp.x;
          position.y = temp.y;
          found = 1;
          break;
        case 0:
          switch (i) {
          case 0:
            temp.x--;
            temp.y++;
            break;
          case 1:
            temp.y = temp.y - 2;
            break;
          case 2:
            logmsg("ERROR: Stuck", DB);
            exit(1);
          }
        }
      }
    } else if (dirX > 0 && dirY < 0) {
      temp.x++;
      for (i = 0; i < 3 && !found; i++) {
        switch (isFree(mapptr, temp)) {
        case 1:
          incTrafficAt(temp);
          position.x = temp.x;
          position.y = temp.y;
          found = 1;
          break;
        case 0:
          switch (i) {
          case 0:
            temp.x--;
            temp.y--;
            break;
          case 1:
            temp.y = temp.y + 2;
            break;
          case 2:
            logmsg("ERROR: Stuck", RUNTIME);
            exit(1);
          }
        }
      }
    } else if (dirX < 0 && dirY > 0) {
      temp.x--;
      for (i = 0; i < 3 && !found; i++) {
        switch (isFree(mapptr, temp)) {
        case 1:
          incTrafficAt(temp);
          position.x = temp.x;
          position.y = temp.y;
          found = 1;
          break;
        case 0:
          switch (i) {
          case 0:
            temp.x++;
            temp.y++;
            break;
          case 1:
            temp.y = temp.y - 2;
            break;
          case 2:
            logmsg("ERROR: Stuck", RUNTIME);
            exit(1);
          }
        }
      }
    } else if (dirX < 0 && dirY < 0) {
      temp.x--;
      for (i = 0; i < 3 && !found; i++) {
        switch (isFree(mapptr, temp)) {
        case 1:
          incTrafficAt(temp);
          position.x = temp.x;
          position.y = temp.y;
          found = 1;
          break;
        case 0:
          switch (i) {
          case 0:
            temp.x++;
            temp.y--;
            break;
          case 1:
            temp.y = temp.y + 2;
            break;
          case 2:
            logmsg("ERROR: Stuck", RUNTIME);
            exit(1);
          }
        }
      }
    } /*END-else-ifs*/

    t1 = (long)(rand() % (timensec_max - timensec_min)) + timensec_min;
    if (t1 > 999999999L) {
      t2 = floor(t1 / 1000000000L);
      t1 = t1 - 1000000000L * t2;
      transit.tv_sec = t2;
    }
    transit.tv_nsec = t1;
    nanosleep(&transit, NULL);
  } /*END-while*/
}

void incTrafficAt(Point p) {
  /*wait mutex*/
  (*mapptr)[p.x][p.y].traffic++;
  logmsg("Incrementato traffico in", DB);
  if (DEBUG)
    printf("\t(%d,%d)\n", p.x, p.y);
  /*signal mutex*/
}

void logmsg(char *message, enum Level l) {
  if (l <= DEBUG) {
    printf("[taxi-%d] %s\n", getpid(), message);
  }
}

void SIGINThandler(int sig) {
  logmsg("Finishing up", DB);
  shmdt(mapptr);
  shmdt(sourcesList_ptr);
  logmsg("Graceful exit successful", DB);
  exit(0);
}

Point getNearSource(int *source_id) {
  Point s;
  int n, temp, d = INT_MAX;
  for (n = 0; n < MAX_SOURCES; n++) {
    temp = abs(position.x - (*sourcesList_ptr)[n].x) +
           abs(position.y - (*sourcesList_ptr)[n].y);
    if (d > temp) {
      d = temp;
      s = (*sourcesList_ptr)[n];
      *source_id = n + 1;
    }
  }
  return s;
}
