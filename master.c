#include "general.h"
void logmsg(char *message, enum Level l) {
  if (l <= DEBUG) {
    printf("[master-%d] %s\n", getpid(), message);
  }
}
int main() {
  char *args[2];
  char *envp[1];
  char id_buffer[30];
  int shmid_map;
  key_t shmkey;
  Cell(*mapptr)[][SO_HEIGHT];
  if ((shmkey = ftok("./.gitignore", 'm')) < 0) {
    EXIT_ON_ERROR
  }
  if ((shmid_map = shmget(shmkey, SO_WIDTH * SO_HEIGHT * sizeof(Cell),
                          IPC_CREAT | 0666)) < 0) {
    EXIT_ON_ERROR
  }

  if ((void *)(mapptr = shmat(shmid_map, NULL, 0)) < (void *)0) {
    EXIT_ON_ERROR
  }
  logmsg("Testing Map", DB);
  (*mapptr)[10][1].state = FREE;
  (*mapptr)[2][7].state = FREE;
  (*mapptr)[4][1].state = FREE;
  (*mapptr)[1][1].capacity = 70;
  (*mapptr)[14][5].state = FREE;
  (*mapptr)[4][1].capacity = 50;
  (*mapptr)[3][9].state = FREE;
  logmsg("Ok", DB);
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
  while (wait(NULL) > 0) {
  }
  if (shmctl(shmid_map, IPC_RMID, NULL)) {
    printf("\nError in shmctl: map,\n");
    EXIT_ON_ERROR
  }

  logmsg("Quitting", DB);
  exit(0);
}
