#Name: Marie Chu
#email: mariechu@ucla.edu
#UID: 905116878

CC = gcc

simpsh: simpsh.c
	$(CC) simpsh.c -o $@
clean: 
	rm -f *tar.gz simpsh
check:
	./script
sources = Makefile README script simpsh.c

dist: $(sources)
	tar -czf lab1-905116878.tar.gz $(sources)
