yash: shell.o forknexecute.o parser.o jobcontrol.o
	gcc -o yash shell.o forknexecute.o parser.o jobcontrol.o -lreadline

shell.o: shell.c
	gcc -c shell.c

forknexecute.o: forknexecute.c
	gcc -c forknexecute.c

parser.o: parser.c
	gcc -c parser.c

jobcontrol.o: jobcontrol.c
	gcc -c jobcontrol.c

clean: 
	rm -f yash *.o