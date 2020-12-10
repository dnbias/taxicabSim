##
# TaxicabSim
#
# @file
# @version 0.1
#
CC = -gcc
CFLAGS = -std=c89 -pedantic

generator: generator.o generator.h
	$(CC) $(CFLAGS) -o generator generator.c


# end
