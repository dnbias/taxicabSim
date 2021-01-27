#ifndef __SOURCE_H_
#define __SOURCE_H_
#include "general.h"
void SIGINThandler(int);

void SIGUSR1handler(int);

void logmsg(char *, enum Level);

#endif
