#ifndef __TAXI_H_
#define __TAXI_H_
#include "general.h"

void logmsg(char *message) {
  int pid;
  if(DEBUG){
    pid = getpid();
    printf("[taxi-%d] %s\n", pid, message);
  }
}

#endif /*__TAXI_H_*/
