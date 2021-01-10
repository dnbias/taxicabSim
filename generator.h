#ifndef __GENERATOR_H_
#define __GENERATOR_H_
#include "general.h"

void parseConf(Config *);

int checkNoAdiacentHoles(Cell (*)[SO_WIDTH][SO_HEIGHT], int, int);

void generateMap(Cell (*)[SO_WIDTH][SO_HEIGHT], Config *);

void printMap(Cell (*)[SO_WIDTH][SO_HEIGHT]);

void logmsg(char *, enum Level);

void SIGINThandler(int);

void ALARMhandler(int);
#endif /* __GENERATOR_H_ */
