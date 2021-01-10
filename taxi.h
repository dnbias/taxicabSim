#ifndef __TAXI_H_
#define __TAXI_H_
#include "general.h"

void SIGINThandler(int);

void logmsg(char *, int);

void incTrafficAt(Point);

void moveTo(Point);

#endif /*__TAXI_H_*/
