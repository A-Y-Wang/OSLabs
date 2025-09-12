yash: shell.o forknexecute.o parser.o
	gcc -o yash shell.o forknexecute.o parser.o -lreadline

shell.o: shell.c
	gcc -c shell.c

forknexecute.o: forknexecute.c
	gcc -c forknexecute.c

parser.o: parser.c
	gcc -c parser.c

clean: 
	rm -f yash *.o