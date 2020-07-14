#NAME: Marie Chu
#EMAIL: mariechu@ucla.edu
#UID: 905116878

CC = gcc
CFLAGS = -g -Wall -Wextra -pthread

build: lab2_add lab2_list

lab2_list: lab2_list.c
	$(CC) $(CFLAGS) SortedList.c lab2_list.c -o lab2_list
lab2_add: lab2_add.c
	$(CC) $(CFLAGS) lab2_add.c -o lab2_add
tests: build
	rm -f lab2_add.csv lab2_list.csv
	./tests.sh
graphs: lab2_list.csv lab2_add.csv
	./lab2_list.gp <lab2_list.csv
	./lab2_add.gp <lab2_add.csv
sources = lab2_add.c SortedList.h SortedList.c lab2_list.c Makefile README lab2_list.gp lab2_add.gp tests.sh lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png lab2_add.csv lab2_list.csv
dist: tests graphs $(sources)
	tar -czf lab2a-905116878.tar.gz $(sources)
clean:
	rm -f *tar.gz lab2_add lab2_list

