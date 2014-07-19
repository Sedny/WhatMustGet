#Makefile

CC = gcc
FLAGS = -Wall -g -O2
objects = my_who my_more

all: my_who my_more


my_who: my_who.o
	$(CC) $(FLAGS) -o $@ $?

my_more: my_more.o
	$(CC) $(FLAGS) -o $@ $?





clean:
	rm -rf $(objects) *.o
