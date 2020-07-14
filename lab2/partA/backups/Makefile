#Name: Marie Chu
#email: mariechu@ucla.edu
#UID: 905116878

CC = gcc
CFLAGS = -g -Wall -Wextra -pthread

build: lab2_add.c lab2_list.c
	$(CC) $(CFLAGS) lab2_add.c -o lab2_add
	$(CC) $(CFLAGS) lab2_list.c -o lab2_list

tests: lab2_add lab2_list
	rm -f lab2_add.csv lab2_list.csv
	./tests.sh	

graphs:
	./lab2_add.gp lab2_add.csv
	./lab2_list.gp lab2_list.csv
sources = lab2_add.c SortedList.h SortedLists.c lab2_list.c Makefile README lab2_add.csv lab2_list.csv lab2_list.gp lab2_add.gp tests.sh

dist: $(sources)
	tar -czf lab2-905116878.tar.gz $(sources)
clean:
	rm -f *tar.gz lab2_add lab2_list *.png *.csv

