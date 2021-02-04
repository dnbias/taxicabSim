#ifndef __SOURCE_H_
#define __SOURCE_H_
#include "general.h"

void handler(int);

int semSyncSource(int);

void logmsg(char *, enum Level);

void userRequest();

#endif
