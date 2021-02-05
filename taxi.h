#ifndef __TAXI_H_
#define __TAXI_H_
#include "general.h"
#include <limits.h>
#include <math.h>

void handler(int);

void logmsg(char *, enum Level);

void incTrafficAt(Point);

void decTrafficAt(Point);

void moveTo(Point);

Point getNearSource();

void checkTimeout();

void printRep();

typedef struct {
  int distance;
  int maxDistanceInTrip;
  struct timeval maxTimeInTrip;
  int clients;
  int tripsSuccess;
  int abort;
} taxiData;

typedef struct {
  long type;
  taxiData data;
} dataMessage;

#endif /*__TAXI_H_*/
