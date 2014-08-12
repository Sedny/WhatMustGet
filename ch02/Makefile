#Makefile

CC = gcc
FLAGS = -Wall -g -O2
objects = my_who my_more my_whoami my_cp

all: my_who my_more my_whoami my_cp

my_who: my_who.o
	$(CC) $(FLAGS) -o $@ $?

my_more: my_more.o
	$(CC) $(FLAGS) -o $@ $?

my_whoami: my_whoami.o
	$(CC) $(FLAGS) -o $@ $?

my_cp: my_cp.o
	$(CC) $(FLAGS) -o $@ $?


clean:
	rm -rf $(objects) *.o
