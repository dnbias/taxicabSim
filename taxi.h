#ifndef __TAXI_H_
#define __TAXI_H_
#include "general.h"
#include <limits.h>

void SIGINThandler(int);

void logmsg(char *, enum Level);

void incTrafficAt(Point);

void moveTo(Point);

Point getNearSource();

#endif /*__TAXI_H_*/
