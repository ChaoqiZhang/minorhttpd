
minorhttpd: main.o daemon.o communicate.o request.o solve.o response.o global.o conf.o
	cc -o minorhttpd main.o daemon.o communicate.o request.o solve.o response.o global.o conf.o -lpthread

global.o: global.c
	cc -c global.c

conf.o : conf.c
	cc -c conf.c

main.o : main.c
	cc -c main.c

solve.o : solve.c request.h solve.h 
	cc -c solve.c

response.o : response.c header.h request.h
	cc -c response.c

request.o : request.c daemon.h request.h 
	cc -c request.c -lpthread

daemon.o : daemon.c daemon.h
	cc -c daemon.c

communicate.o : communicate.c communicate.h daemon.h
	cc -c communicate.c

.PHONY : clean

clean : 
	-rm *.o 
