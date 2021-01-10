#ifndef __GENERATOR_H_
#define __GENERATOR_H_
#include "general.h"

void parseConf(Config *);

int checkNoAdiacentHoles(Cell (*)[SO_WIDTH][SO_HEIGHT], int, int);

void generateMap(Cell (*)[SO_WIDTH][SO_HEIGHT], Config *);

int isFree(Cell (*)[SO_WIDTH][SO_HEIGHT], Point);

void printMap(Cell (*)[SO_WIDTH][SO_HEIGHT]);

void logmsg(char *, Level);

void SIGINThandler(int);

void ALARMhandler(int);
#endif /* __GENERATOR_H_ */
