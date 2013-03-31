
minorhttpd: main.o daemon.o communicate.o request.o
	cc -o minorhttpd main.o daemon.o communicate.o request.o -lpthread

request.o : request.c daemon.h request.h config.h
	cc -c request.c -lpthread

main.o : main.c
	cc -c main.c

daemon.o : daemon.c daemon.h
	cc -c daemon.c

communicate.o : communicate.c communicate.h daemon.h
	cc -c communicate.c

.PHONY : clean

clean : 
	-rm *.o 
