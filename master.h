#ifndef __MASTER_H_
#define __MASTER_H_
#include "general.h"
#include <stdlib.h>
#include <string.h>
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
  int requests;
} sourceMessage;

typedef struct {
  long type;
  taxiData data;
} dataMessage;

typedef struct {
  int requests;
  int trips;
  int tripsSuccess;
  int tripsNotServed;
  int maxTrips;
  long tripsWinner;
  struct timeval maxTime;
  long timeWinner;
  int maxDistance;
  long distanceWinner;
  int topCells;
  Point (*cellsWinner)[];
} Data;

void cellsData(Cell (*)[][SO_HEIGHT], int);

void updateData(long, taxiData *);

void printMap(Cell (*)[][SO_HEIGHT]);

void printReport(Cell (*)[][SO_HEIGHT]);

void logmsg(char *, enum Level);

void handler(int);
#endif
