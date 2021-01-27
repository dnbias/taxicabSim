#include "taxi.h"
int shmid, source_id, master_qid, sem_idR, sem_idW;
Cell (*mapptr)[][SO_HEIGHT];
Point (*sourcesList_ptr)[MAX_SOURCES];
Point position;
int qid, timensec_min, timensec_max, timeout;
taxiData data;
dataMessage data_msg;

int main(int argc, char **argv) {

  key_t shmkey, qkey, semkeyR, semkeyW;
  Message msg;
  const struct timespec nsecs[] = {0, 500000000L};
  struct sigaction act;

  /************INIT************/
  logmsg("Init...", DB);
  memset(&act, 0, sizeof(act));
  act.sa_handler = handler;

  sigaction(SIGINT, &act, 0);
  sigaction(SIGALRM, &act, 0);
  sigaction(SIGUSR1, &act, 0);
  sigaction(SIGUSR2, &act, 0);

  if ((shmkey = ftok("./makefile", 'b')) < 0) {
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

  if ((shmkey = ftok("./makefile", 'm')) < 0) {
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
  /*  queue for comunication with sources */
  if ((qkey = ftok("./makefile", 'q')) < 0) {
    EXIT_ON_ERROR
  }
  if ((qid = msgget(qkey, 0644)) < 0) {
    EXIT_ON_ERROR
  }
  /*  queue for comunication with master */
  if ((qkey = ftok("./makefile", 'd')) < 0) {
    EXIT_ON_ERROR
  }
  if ((master_qid = msgget(qkey, 0644)) < 0) {
    EXIT_ON_ERROR
  }

  if ((semkeyR = ftok("./makefile", 'r')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((sem_idR = semget(semkeyR, 0, 0)) < 0) {
    printf("semget error\n");
    EXIT_ON_ERROR
  }
  if ((semkeyW = ftok("./makefile", 'w')) < 0) {
    printf("ftok error\n");
    EXIT_ON_ERROR
  }
  if ((sem_idW = semget(semkeyW, 0, 0)) < 0) {
    printf("semget error\n");
    EXIT_ON_ERROR
  }

  sscanf(argv[1], "%d", &position.x);
  sscanf(argv[2], "%d", &position.y);
  sscanf(argv[3], "%d", &timensec_min);
  sscanf(argv[4], "%d", &timensec_max);
  sscanf(argv[5], "%d", &timeout);
  logmsg("Init Finished", DB);
  srand(time(NULL) + getpid());
  data_msg.type = getpid();
  data_msg.data.distance = 0;
  data_msg.data.maxDistanceInTrip = 0;
  data_msg.data.clients = 0;
  data_msg.data.tripsSuccess = 0;
  /************END-INIT************/

  incTrafficAt(position);
  while (1) {
    logmsg("Going to Nearest Source", DB);
    moveTo(getNearSource(&source_id));
    msgrcv(qid, &msg, sizeof(Point), source_id, 0);
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
  Point temp;
  logmsg("Moving to destination:", DB);
  if (DEBUG)
    printf("[taxi-%ld]--->(%d,%d)\n", getpid(), dest.x, dest.y);
  oldDistance = data_msg.data.distance;
  while (position.x != dest.x || position.y != dest.y) {
    dirX = dest.x - position.x;
    dirY = dest.y - position.y;
    temp.x = position.x;
    temp.y = position.y;
    found = 0;
    if ((abs(dirX) <= 1 && dirY == 0) || (dirX == 0 && abs(dirY) <= 1)) {
      incTrafficAt(temp);
      decTrafficAt(position);
      position.x = dest.x;
      position.y = dest.y;
      found = 1;
    } else if (dirX >= 0 && dirY >= 0) {
      temp.x++;
      for (i = 0; i < 3 && !found; i++) {
        switch (isFree(mapptr, temp)) {
        case 1:
          incTrafficAt(temp);
          decTrafficAt(position);
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
            break;
          }
        }
      }
    } else if (dirX >= 0 && dirY <= 0) {
      temp.x++;
      for (i = 0; i < 3 && !found; i++) {
        switch (isFree(mapptr, temp)) {
        case 1:
          incTrafficAt(temp);
          decTrafficAt(position);
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
            break;
          }
        }
      }
    } else if (dirX <= 0 && dirY >= 0) {
      temp.x--;
      for (i = 0; i < 3 && !found; i++) {
        switch (isFree(mapptr, temp)) {
        case 1:
          incTrafficAt(temp);
          decTrafficAt(position);
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
            break;
          }
        }
      }
    } else if (dirX <= 0 && dirY <= 0) {
      temp.x--;
      for (i = 0; i < 3 && !found; i++) {
        switch (isFree(mapptr, temp)) {
        case 1:
          incTrafficAt(temp);
          decTrafficAt(position);

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
            break;
          }
        }
      }
    } /*END-else-ifs*/
    logmsg("looping to", DB);
    if (DEBUG)
      printf("[taxi-%ld]--->(%d,%d)\n", getpid(), dest.x, dest.y);

    t1 = (long)(rand() % (timensec_max - timensec_min)) + timensec_min;
    if (t1 > 999999999L) {
      t2 = floor(t1 / 1000000000L);
      t1 = t1 - 1000000000L * t2;
      transit.tv_sec = t2;
    }
    transit.tv_nsec = t1;
    nanosleep(&transit, NULL);
    data_msg.data.distance++;
  } /*END-while*/
  if ((data_msg.data.distance - oldDistance) >
      data_msg.data.maxDistanceInTrip) {
    data_msg.data.maxDistanceInTrip = data_msg.data.distance - oldDistance;
  }
  logmsg("Arrived in", DB);
  if (DEBUG)
    printf("[taxi-%ld]--->(%d,%d)\n", getpid(), dest.x, dest.y);
}

void incTrafficAt(Point p) {
  /*wait mutex*/
  /*if(scrivi(p) < 0){
        EXIT_ON_ERROR
        }*/
  (*mapptr)[p.x][p.y].traffic++;
  /*if(release(p) < 0){
        EXIT_ON_ERROR
        }*/
  if (DEBUG)
    printf("[taxi-%ld]->(%d,%d)\n", getpid(), p.x, p.y);
  /*signal mutex*/
}

void decTrafficAt(Point p) { (*mapptr)[p.x][p.y].traffic--; }

void logmsg(char *message, enum Level l) {
  if (l <= DEBUG) {
    printf("[taxi-%d] %s\n", getpid(), message);
  }
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
/*FUNZIONI PER CONTROLLARE SEMAFORI*/

int leggi(Point p) {
  struct sembuf writer[2], reader;
  writer[0].sem_num = p.y * SO_WIDTH + p.x;
  writer[0].sem_op = 0;
  writer[0].sem_flg = 0;
  writer[1].sem_num = p.y * SO_WIDTH + p.x;
  writer[1].sem_op = 1;
  writer[1].sem_flg = 0;
  reader.sem_num = p.y * SO_WIDTH + p.x;
  reader.sem_op = 1;
  reader.sem_flg = 0;
  return semop(sem_idW, writer, 2) + semop(sem_idR, &reader, 1);
}

int scrivi(Point p) {
  /*semctl(sem_idW, p.y*SO_WIDTH + p.x, GETZCNT, &idR);*/
  struct sembuf writer[2], reader[2];
  writer[0].sem_num = p.y * SO_WIDTH + p.x;
  writer[0].sem_op = 0;
  writer[0].sem_flg = 0;
  reader[0].sem_num = p.y * SO_WIDTH + p.x;
  reader[0].sem_op = 0;
  reader[0].sem_flg = 0;
  writer[1].sem_num = p.y * SO_WIDTH + p.x;
  writer[1].sem_op = 1;
  writer[1].sem_flg = 0;
  reader[1].sem_num = p.y * SO_WIDTH + p.x;
  reader[1].sem_op = 1;
  reader[1].sem_flg = 0;
  return semop(sem_idW, writer, 1) + semop(sem_idR, reader, 2);
}

int releaseW(Point p) {
  struct sembuf releaseW;
  releaseW.sem_num = p.y * SO_WIDTH + p.x;
  releaseW.sem_op = -1;
  releaseW.sem_flg = IPC_NOWAIT;
  return semop(sem_idW, &releaseW, 1);
}

int release(Point p) {
  struct sembuf releaseW, releaseR;
  releaseW.sem_num = p.y * SO_WIDTH + p.x;
  releaseW.sem_op = -1;
  releaseW.sem_flg = IPC_NOWAIT;
  releaseR.sem_num = p.y * SO_WIDTH + p.x;
  releaseR.sem_op = -1;
  releaseR.sem_flg = IPC_NOWAIT;
  return semop(sem_idW, &releaseW, 1) + semop(sem_idR, &releaseR, 1);
}

int releaseR(Point p) {
  struct sembuf releaseR;
  releaseR.sem_num = p.y * SO_WIDTH + p.x;
  releaseR.sem_op = -1;
  releaseR.sem_flg = IPC_NOWAIT;
  return semop(sem_idR, &releaseR, 1);
}

void handler(int sig) {
  switch (sig) {
  case SIGINT:
    logmsg("Finishing up", DB);
    shmdt(mapptr);
    shmdt(sourcesList_ptr);
    msgsnd(master_qid, &data_msg, sizeof(taxiData), 0);
    logmsg("Graceful exit successful", DB);
    exit(0);
  case SIGUSR1:
    break;
  }
}
