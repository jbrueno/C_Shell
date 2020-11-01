
CC=gcc
#CC=gcc -Wall

mysh: shell.o get_path.o main.c 
	$(CC) -g main.c shell.o get_path.o -o mysh
#	$(CC) -g main.c sh.o get_path.o bash_getcwd.o -o mysh

shell.o: shell.c shell.h
	$(CC) -g -c shell.c

get_path.o: get_path.c get_path.h
	$(CC)  -g -c get_path.c

clean:
	rm -rf shell.o get_path.o mysh
