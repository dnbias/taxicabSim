#include "taxi.h"

int shmid, source_id, master_qid, writers, mutex, sem, semSource;
Cell (*mapptr)[][SO_HEIGHT];
Point (*sourcesList_ptr)[];
int *readers;
Point position;
int qid, timensec_min, timensec_max, timeout, n_sources;
/*taxiData data;*/
dataMessage data_msg;
struct timeval timer;

int main(int argc, char **argv) {
  int received;
  key_t shmkey, qkey, key;
  Message msg;
  const struct timespec nsecs[] = {0, 500000000L};
  struct sigaction act;

  /************INIT************/
  logmsg("Init...", DB);
  memset(&act, 0, sizeof(act));
  act.sa_handler = handler;

  sigaction(SIGINT, &act, 0);
  sigaction(SIGALRM, &act, 0);
  sigaction(SIGQUIT, &act, 0);
  sigaction(SIGUSR1, &act, 0);
  sigaction(SIGUSR2, &act, 0);
  sigaction(SIGTSTP, &act, 0);

  /* Shared Memory */
  if ((shmkey = ftok("./makefile", 'b')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((shmid = shmget(shmkey, 0, 0644)) < 0) {
    EXIT_ON_ERROR
  }
  if ((void *)(sourcesList_ptr = shmat(shmid, NULL, 0)) < (void *)0) {
    EXIT_ON_ERROR
  }

  if ((shmkey = ftok("./makefile", 'm')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((shmid = shmget(shmkey, 0, 0644)) < 0) {
    EXIT_ON_ERROR
  }
  if ((void *)(mapptr = shmat(shmid, NULL, 0)) < (void *)0) {
    logmsg("ERROR shmat - mapptr", RUNTIME);
    EXIT_ON_ERROR
  }

  if ((shmkey = ftok("./makefile", 'z')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((shmid = shmget(shmkey, 0, 0666)) < 0) {
    EXIT_ON_ERROR
  }
  if ((void *)(readers = shmat(shmid, NULL, 0)) < (void *)0) {
    logmsg("ERROR shmat - readers", RUNTIME);
    EXIT_ON_ERROR
  }
  logmsg("Shared Memory Init OK", DB);
  /*  queue for comunication with master */
  if ((qkey = ftok("./makefile", 'd')) < 0) {
    EXIT_ON_ERROR
  }
  if ((master_qid = msgget(qkey, 0644)) < 0) {
    EXIT_ON_ERROR
  }
  logmsg("Queue Init OK", DB);
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
  logmsg("Semaphores Init OK", DB);
  semSync(sem);
  logmsg("Sync OK", DB);
  /*  queue for comunication with sources */
  if ((qkey = ftok("./makefile", 'q')) < 0) {
    EXIT_ON_ERROR
  }
  if ((qid = msgget(qkey, 0644)) < 0) {
    EXIT_ON_ERROR
  }

  sscanf(argv[1], "%d", &position.x);
  sscanf(argv[2], "%d", &position.y);
  sscanf(argv[3], "%d", &timensec_min);
  sscanf(argv[4], "%d", &timensec_max);
  sscanf(argv[5], "%d", &timeout);
  sscanf(argv[6], "%d", &n_sources);
  srand(time(NULL) + getpid());
  data_msg.type = getpid();
  data_msg.data.distance = 0;
  data_msg.data.maxDistanceInTrip = 0;
  data_msg.data.maxTimeInTrip.tv_sec = 0;
  data_msg.data.maxTimeInTrip.tv_usec = 0;
  data_msg.data.clients = 0;
  data_msg.data.tripsSuccess = 0;
  data_msg.data.abort = 0;
  logmsg("Init Finished", DB);
  /************END-INIT************/

  incTrafficAt(position);
  while (1) {
    gettimeofday(&timer, NULL);
    logmsg("Going to Nearest Source", DB);
    moveTo(getNearSource(&source_id));
    received = 0;

    while (!received) {
      logmsg("Listening for call on:", DB);
      if (DEBUG)
        printf("Source[%d](%d,%d)\n", source_id, position.x, position.y);
      if (msgrcv(qid, &msg, sizeof(Point), source_id, 0) < 0) {
        checkTimeout();
        logmsg("Timeout passed, waiting for message", SILENCE);
      } else
        received = 1;
    }
    data_msg.data.clients++;
    logmsg("Going to destination", DB);
    moveTo(msg.destination);
    data_msg.data.tripsSuccess++;
  }
}

void moveTo(Point dest) { /*pathfinding*/
  int dirX, dirY, i, found, oldDistance;
  long t1, t2;
  struct timespec transit;
  struct timeval start, end;
  Point temp, oldPos;
  logmsg("Moving to destination:", SILENCE);
  if (0)
    printf("[taxi-%d]--->(%d,%d)\n", getpid(), dest.x, dest.y);
  oldDistance = data_msg.data.distance;
  gettimeofday(&start, NULL);
  gettimeofday(&timer, NULL);
  oldPos.x = position.x;
  oldPos.y = position.y;
  while (position.x != dest.x || position.y != dest.y) {
    checkTimeout();
    t2 = 0;

    if (0)
      printf("[taxi-%d] pos: (%d,%d)\n", getpid(), position.x, position.y);
    if (0)
      printf("[taxi-%d]--->(%d,%d)\n", getpid(), dest.x, dest.y);
    dirX = dest.x - position.x;
    dirY = dest.y - position.y;
    temp.x = position.x;
    temp.y = position.y;
    found = 0;
    if (0)
      printf("[taxi-%d] DirX: %d DirY %d\n", getpid(), dirX, dirY);
    if (abs(dirX + dirY) == 1) {
      incTrafficAt(dest);
      decTrafficAt(position);
      position.x = dest.x;
      position.y = dest.y;
      if (0)
        printf("[taxi-%d] arrived at: (%d,%d)\n", getpid(), dest.x, dest.y);
      break;
    }
    if (dirX == 0 && dirY > 0 && (oldPos.y != position.y + 1) &&
        (*mapptr)[temp.x][temp.y + 1].state == FREE) {
      temp.y++;
      while (!found) {
        checkTimeout();
        if (canTransit(temp)) {
          incTrafficAt(temp);
          decTrafficAt(position);
          oldPos.x = position.x;
          oldPos.y = position.y;
          position.x = temp.x;
          position.y = temp.y;
          found = 1;
        }
      }
    } else if (dirX == 0 && dirY < 0 && (oldPos.y != position.y - 1) &&
               (*mapptr)[temp.x][temp.y - 1].state == FREE) {
      temp.y--;
      while (!found) {
        checkTimeout();
        if (canTransit(temp)) {
          incTrafficAt(temp);
          decTrafficAt(position);
          oldPos.x = position.x;
          oldPos.y = position.y;
          position.x = temp.x;
          position.y = temp.y;
          found = 1;
        }
      }
    } else if (dirY == 0 && dirX > 0 && (oldPos.x != position.x + 1) &&
               (*mapptr)[temp.x + 1][temp.y].state == FREE) {
      temp.x++;
      while (!found) {
        checkTimeout();
        if (canTransit(temp)) {
          incTrafficAt(temp);
          decTrafficAt(position);
          oldPos.x = position.x;
          oldPos.y = position.y;
          position.x = temp.x;
          position.y = temp.y;
          found = 1;
        }
      }
    } else if (dirY == 0 && dirX < 0 && (oldPos.x != position.x - 1) &&
               (*mapptr)[temp.x - 1][temp.y].state == FREE) {
      temp.x--;
      while (!found) {
        checkTimeout();
        if (canTransit(temp)) {
          incTrafficAt(temp);
          decTrafficAt(position);
          oldPos.x = position.x;
          oldPos.y = position.y;
          position.x = temp.x;
          position.y = temp.y;
          found = 1;
        }
      }
    } else if (dirX >= 0 && dirY >= 0) {
      for (i = 0; i < 5 && !found; i++) {
        switch (i) {
        case 0:
          temp.x++;
          break;
        case 1:
          temp.y++;
          break;
        case 2:
          temp.x--;
          break;
        case 3:
          temp.y--;
          break;
        case 4:
          i = -1;
          continue;
        }
        checkTimeout();
        if (oldPos.x == temp.x && oldPos.y == temp.y) {
          temp.x = position.x;
          temp.y = position.y;
          continue;
        } else {
          if (!canTransit(temp))
            continue;
          incTrafficAt(temp);
          decTrafficAt(position);
          oldPos.x = position.x;
          oldPos.y = position.y;
          position.x = temp.x;
          position.y = temp.y;
          found = 1;
        }
      }
    } else if (dirX >= 0 && dirY <= 0) {
      for (i = 0; i < 5 && !found; i++) {
        switch (i) {
        case 0:
          temp.x++;
          break;
        case 1:
          temp.y--;
          break;
        case 2:
          temp.x--;
          break;
        case 3:
          temp.y++;
          break;
        case 4:
          i = -1;
          continue;
        }
        checkTimeout();
        if (oldPos.x == temp.x && oldPos.y == temp.y) {
          temp.x = position.x;
          temp.y = position.y;
          continue;
        } else {
          if (!canTransit(temp))
            continue;
          incTrafficAt(temp);
          decTrafficAt(position);
          oldPos.x = position.x;
          oldPos.y = position.y;
          position.x = temp.x;
          position.y = temp.y;
          found = 1;
        }
      }
    } else if (dirX <= 0 && dirY >= 0) {
      for (i = 0; i < 5 && !found; i++) {
        switch (i) {
        case 0:
          temp.y++;
          break;
        case 1:
          temp.y--;
          break;
        case 2:
          temp.x--;
          break;
        case 3:
          temp.x++;
          break;
        case 4:
          i = -1;
          continue;
        }
        checkTimeout();
        if (oldPos.x == temp.x && oldPos.y == temp.y) {
          temp.x = position.x;
          temp.y = position.y;
          continue;
        } else {
          if (!canTransit(temp))
            continue;
          incTrafficAt(temp);
          decTrafficAt(position);
          oldPos.x = position.x;
          oldPos.y = position.y;
          position.x = temp.x;
          position.y = temp.y;
          found = 1;
        }
      }
    } else if (dirX <= 0 && dirY <= 0) {
      for (i = 0; i < 5 && !found; i++) {
        switch (i) {
        case 0:
          temp.x--;
          break;
        case 1:
          temp.y--;
          break;
        case 2:
          temp.x++;
          break;
        case 3:
          temp.y++;
          break;
        case 4:
          i = -1;
          continue;
        }
        checkTimeout();
        if (oldPos.x == temp.x && oldPos.y == temp.y) {
          temp.x = position.x;
          temp.y = position.y;
          continue;
        } else {
          if (!canTransit(temp))
            continue;
          incTrafficAt(temp);
          decTrafficAt(position);
          oldPos.x = position.x;
          oldPos.y = position.y;
          position.x = temp.x;
          position.y = temp.y;
          found = 1;
        }
      }
    } /*END-else-ifs*/

    t1 = (long)(rand() % (timensec_max - timensec_min)) + timensec_min;
    if (t1 > 999999999L) {
      t2 = floor(t1 / 1000000000L);
      t1 = t1 - 1000000000L * t2;
    }
    transit.tv_sec = t2;
    transit.tv_nsec = t1;
    nanosleep(&transit, NULL);
    (*mapptr)[position.x][position.y].visits++;
    data_msg.data.distance++;
    gettimeofday(&timer, NULL);
  } /*END-while*/
  gettimeofday(&end, NULL);
  data_msg.data.maxTimeInTrip.tv_sec = (end.tv_sec - start.tv_sec);
  data_msg.data.maxTimeInTrip.tv_usec = labs(end.tv_usec - start.tv_usec);
  if ((data_msg.data.distance - oldDistance) >
      data_msg.data.maxDistanceInTrip) {
    data_msg.data.maxDistanceInTrip = data_msg.data.distance - oldDistance;
  }
  logmsg("Arrived in", DB);
  if (DEBUG)
    printf("[taxi-%d]--->(%d,%d)\n", getpid(), dest.x, dest.y);
  gettimeofday(&timer, NULL);
}

int canTransit(Point p) {
  int r;
  if (!(p.x >= 0 && p.x < SO_WIDTH && p.y >= 0 && p.y < SO_HEIGHT))
    return 0;
  lock(mutex);
  *readers++;
  if (*readers == 1)
    semWait(p, writers);
  unlock(mutex);
  r = isFree(mapptr, p) &&
      (*mapptr)[p.x][p.y].traffic < (*mapptr)[p.x][p.y].capacity;
  lock(mutex);
  *readers--;
  if (*readers == 0)
    semSignal(p, writers);
  unlock(mutex);
  return r;
}

void incTrafficAt(Point p) {
  if (!(p.x >= 0 && p.x < SO_WIDTH && p.y >= 0 && p.y < SO_HEIGHT))
    EXIT_ON_ERROR
  semWait(p, writers);
  (*mapptr)[p.x][p.y].traffic++;
  semSignal(p, writers);
  if (0)
    printf("[taxi-%d]->(%d,%d)\n", getpid(), p.x, p.y);
}

void decTrafficAt(Point p) {
  if (!(p.x >= 0 && p.x < SO_WIDTH && p.y >= 0 && p.y < SO_HEIGHT))
    EXIT_ON_ERROR
  semWait(p, writers);
  (*mapptr)[p.x][p.y].traffic--;
  semSignal(p, writers);
}

void logmsg(char *message, enum Level l) {
  if (l <= DEBUG) {
    printf("[taxi-%d] %s\n", getpid(), message);
  }
}

Point getNearSource(int *source_id) {
  Point s;
  int n, temp, d = INT_MAX;
  for (n = 0; n < n_sources; n++) {
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

void checkTimeout() {
  struct timeval elapsed;
  int s, u, n;
  logmsg("Timeout check", SILENCE);
  gettimeofday(&elapsed, NULL);
  s = elapsed.tv_sec - timer.tv_sec;
  u = elapsed.tv_usec - timer.tv_usec;
  n = s * 1000000000 + (u * 1000);
  if (n >= timeout) {
    logmsg("Timedout", DB);
    (*mapptr)[position.x][position.y].traffic--;
    data_msg.data.abort++;
    logmsg("Finishing up", SILENCE);
    shmdt(mapptr);
    shmdt(sourcesList_ptr);
    shmdt(readers);
    msgsnd(master_qid, &data_msg, sizeof(taxiData), 0);
    logmsg("Graceful exit successful", DB);
    printRep();
    kill(getppid(), SIGUSR1);
    exit(0);
  } else
    return;
}

void handler(int sig) {
  switch (sig) {
  case SIGINT:
    logmsg("Finishing up", SILENCE);
    (*mapptr)[position.x][position.y].traffic--;
    shmdt(mapptr);
    shmdt(sourcesList_ptr);
    shmdt(readers);
    msgsnd(master_qid, &data_msg, sizeof(taxiData), IPC_NOWAIT);
    logmsg("Graceful exit successful", DB);
    printRep();

    exit(0);
  case SIGQUIT:
    logmsg("SIGQUIT", DB);
    break;
  case SIGALRM:
    raise(SIGINT);
    break;
  case SIGUSR1:
    logmsg("Received SIGUSR1", DB);
    break;
  case SIGSTOP:
    break;
  case SIGUSR2:
    logmsg("Received SIGUSR2", DB);
    break;
  }
}

void printRep() {
  if (DEBUG)
    printf(
        ANSI_COLOR_RED
        "\ntaxiN°: %ld, distance: %i, MAXdistance: %i, MAXtimeintrips: %ld "
        "ms,\n"
        "clients: %i, tripsSuccess: %i, abort: %i;\n\n" ANSI_COLOR_RESET,
        data_msg.type, data_msg.data.distance, data_msg.data.maxDistanceInTrip,
        (data_msg.data.maxTimeInTrip.tv_sec * 1000 +
         data_msg.data.maxTimeInTrip.tv_usec / 1000),
        data_msg.data.clients, data_msg.data.tripsSuccess, data_msg.data.abort);
}
