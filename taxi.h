#ifndef __TAXI_H_
#define __TAXI_H_
#include "general.h"
#include <limits.h>
#include <math.h>

void SIGINThandler(int);

void SIGUSR1handler(int);

void logmsg(char *, enum Level);

void incTrafficAt(Point);

void moveTo(Point);

Point getNearSource();

typedef struct {
  int distance;
  int maxDistanceInTrip;
  int clients;
  int tripsSuccess;
} taxiData;

typedef struct {
  long type;
  taxiData data;
} dataMessage;

#endif /*__TAXI_H_*/
