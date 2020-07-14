#Name: Marie Chu
#email: mariechu@ucla.edu
#UID: 905116878

CC = gcc

simpsh: simpsh.c
	$(CC) simpsh.c -o $@
clean: 
	rm -f *tar.gz simpsh
check:
	./test.sh
sources = Makefile README test.sh simpsh.c grading_helper

dist: $(sources)
	tar -czf lab1-905116878.tar.gz $(sources)
