erg: main.o processes.o
	gcc main.o processes.o -lpthread -o erg
main.o: main.c header.h
	gcc -c main.c
processes.o: processes.c header.h 
	gcc -c processes.c
clean:
	rm erg main.o processes.o
