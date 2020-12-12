##
# TaxicabSim
#
# @file
# @version 0.1
#
CC = -gcc
CFLAGS = -std=c89 -pedantic -g

generator: generator.o generator.h general.h taxi
	$(CC) $(CFLAGS) -o generator generator.c

taxi: taxi.o taxi.h general.h
	$(CC) $(CFLAGS) -o taxi taxi.c

# end
