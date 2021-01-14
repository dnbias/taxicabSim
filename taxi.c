#include "taxi.h"

void *mapptr, *sources_ptr;
Point position;
int qid;

int main(int argc, char **argv) {
  int shmid, sem_idR, sem_idW;
  key_t shmkey, qkey, semkeyR, semkeyW;
  Message msg;

  logmsg("Init...", DB);
  if (DEBUG)
    sleep(1);

  /************INIT************/
  if ((shmkey = ftok("makefile", 's')) < 0) {
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
  if ((sources_ptr = shmat(shmid, NULL, 0)) < (void *)0) {
    EXIT_ON_ERROR
  }

  if ((shmkey = ftok("makefile", 'm')) < 0) {
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
  if ((mapptr = shmat(shmid, NULL, 0)) < (void *)0) {
    EXIT_ON_ERROR
  }
  if ((qkey = ftok("makefile", 'q')) < 0) {
    EXIT_ON_ERROR
  }
  if ((qid = msgget(qkey, 0644)) < 0) {
    EXIT_ON_ERROR
  }


  if((semkeyR = ftok("makefile", 'r')) < 0){
  	printf("ftok error\n");
  	EXIT_ON_ERROR
  }
  if((sem_idR = semget(semkeyR, 0, 0)) < 0){
  	printf("semget error\n");
  	EXIT_ON_ERROR
  }
  if((semkeyW = ftok("makefile", 'w')) < 0){
    printf("ftok error\n");
  	EXIT_ON_ERROR
  }
  if((sem_idW = semget(semkeyW, 0, 0)) < 0){
    printf("semget error\n");
  	EXIT_ON_ERROR
  }


  signal(SIGINT, SIGINThandler);
  sscanf(argv[1], "%d", &position.x);
  sscanf(argv[2], "%d", &position.y);
  logmsg("Init Finished", DB);
  /************END-INIT************/

  incTrafficAt(position);

  while (1) {
    msgrcv(qid, &msg, sizeof(Point), 0, 0);
    logmsg("Going to Nearest Source", DB);
    /* moveTo(getNearSource()); ********** TODO *********/
    moveTo(getNearSource());
    logmsg("Going to destination", DB);
    moveTo(msg.destination);
  }
}

void moveTo(Point p) { /*pathfinding*/
  if (DEBUG)
    usleep(5000000);
  logmsg("Moving to", DB);
  incTrafficAt(p);
}

void incTrafficAt(Point p) {
  /*wait mutex*/
  ((Cell(*)[SO_WIDTH][SO_HEIGHT])mapptr)[p.x][p.y]->traffic++;
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
  shmdt(sources_ptr);
  logmsg("Graceful exit successful", DB);
  exit(0);
}

Point getNearSource() {
  Point s;
  int n, temp, d = 0;
  for (n = 0; n < MAX_SOURCES; n++) {
    temp = abs(position.x - ((Point(*)[MAX_SOURCES])sources_ptr)[n]->x) +
           abs(position.y - ((Point(*)[MAX_SOURCES])sources_ptr)[n]->y);
    if (d > temp)
      d = temp;
  }
}
/*FUNZIONI PER CONTROLLARE SEMAFORI*/
int semReserveUse(Point p, int sem_id){
	struct sembuf sops[2];
	sops[0].sem_num = p.y*SO_WIDTH + p.x;
	sops[0].sem_op = 0;
	sops[0].sem_flg = 0;
	sops[1].sem_num = p.y*SO_WIDTH + p.x;
	sops[1].sem_op = 1;
	sops[1].sem_flg = 0;
	return semop(sem_id, sops,2);
}

int semRelease(Point p, int sem_id){
	struct sembuf sops;
	sops.sem_num = p.y*SO_WIDTH + p.x;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	return semop(sem_id, &sops,1);
}
