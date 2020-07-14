#NAME: Marie Chu
#EMAIL: mariechu@ucla.edu
#UID: 905116878

CC = gcc
CFLAGS = -g -Wall -Wextra 

lab3a: lab3a.c
	$(CC) $(CFLAGS) lab3a.c -o lab3a

sources = lab3a.c README Makefile ext2_fs.h

dist: 
	tar -czf lab3a-905116878.tar.gz $(sources)
clean:
	rm -f *tar.gz lab3a 

