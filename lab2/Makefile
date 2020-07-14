#NAME: Marie Chu
#EMAIL: mariechu@ucla.edu
#UID: 905116878

CC = gcc
CFLAGS = -g -Wall -Wextra -pthread -lprofiler

lab2_list: lab2_list.c
	$(CC) $(CFLAGS) SortedList.c lab2_list.c -o lab2_list

tests: lab2_list
	rm -f lab2b_list.csv
	./tests.sh

profile: lab2_list
	rm -f profile.out
	LD_PRELOAD=/lib64/libprofiler.so 
	CPUPROFILE=profile.out ./lab2_list --iterations=1000 --threads=12 --sync=s
	/bin/pprof --test ./lab2_list profile.out
	/bin/pprof --list=worker ./lab2_list profile.out

graphs: lab2b_list.csv 
	./lab2_list.gp <lab2b_list.csv
	
sources = SortedList.h SortedList.c lab2_list.c Makefile README lab2_list.gp tests.sh lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png lab2b_list.csv profile.out

dist: tests graphs profile
	tar -czf lab2b-905116878.tar.gz $(sources)
clean:
	rm -f *tar.gz lab2_list 

