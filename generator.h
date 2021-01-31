#ifndef __GENERATOR_H_
#define __GENERATOR_H_
#include "general.h"
#include <stdio.h>

void unblock(int);

void parseConf(Config *);

int checkNoAdiacentHoles(Cell (*)[][SO_HEIGHT], int, int);

void generateMap(Cell (*)[][SO_HEIGHT], Config *);

void printMap(Cell (*)[][SO_HEIGHT]);

void logmsg(char *, enum Level);

void execSource(int);

void execTaxi();

void handler(int);

#endif /* __GENERATOR_H_ */
