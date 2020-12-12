#ifndef __GENERATOR_H_
#define __GENERATOR_H_
#include "general.h"

void logmsg(char *message) {
  int pid;
  pid = getpid();
  printf("[generator-%d] %s\n", pid, message);
}

#endif /* __GENERATOR_H_ */
