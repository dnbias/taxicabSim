##
# TaxicabSim
#
# @file
# @version 0.1
#
CC = -gcc
CFLAGS := -D_POSIX_C_SOURCE -std=c89 -pedantic -g -DDEBUG=1

taxicab: master.o master.h generator source taxi cleanall general.h generator.h source.h taxi.h
	$(CC) $(CFLAGS) -o taxicab master.c

generator: generator.o generator.h general.h
	$(CC) $(CFLAGS) -o generator generator.c

taxi: taxi.o taxi.h general.h functions.o
	$(CC) $(CFLAGS) -o taxi taxi.c functions.c

source: source.o source.h general.h functions.o
	$(CC) $(CFLAGS) -o source source.c functions.c

cleanall:
	rm -f *.o
# end
