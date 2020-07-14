#NAME: Marie Chu
#EMAIL: mariechu@ucla.edu
#ID: 905116878
CC = gcc
CFLAGS = -g -Wall -Wextra

lab0: lab0.c
	$(CC) $(CFLAGS) lab0.c -o $@

check: lab0 script
	./script
	
clean: 
	rm -f *.tar.gz lab0
sources = Makefile README breakpoint.png backtrace.png lab0.c script
dist: $(sources)
	tar -czf lab0-905116878.tar.gz $(sources)

