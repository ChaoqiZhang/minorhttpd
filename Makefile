
minorhttpd: main.o daemon.o communicate.o request.o solve.o response.o global.o conf.o reio.o php_cgi.o
	cc -o minorhttpd main.o daemon.o communicate.o request.o solve.o response.o global.o conf.o reio.o php_cgi.o -lpthread

php_cgi.o: cgi/php_cgi.c request.h
	cc -c cgi/php_cgi.c

global.o: global.c -lpthread
	cc -c global.c

conf.o : conf.c
	cc -c conf.c

main.o : main.c
	cc -c main.c

solve.o : solve.c request.h solve.h 
	cc -c solve.c

response.o : response.c header.h request.h communicate.h
	cc -c response.c -lpthread

request.o : request.c daemon.h request.h 
	cc -c request.c -lpthread

daemon.o : daemon.c daemon.h
	cc -c daemon.c

communicate.o : communicate.c communicate.h daemon.h
	cc -c communicate.c -lpthread

reio.o : reio.c
	cc -c reio.c

.PHONY : clean

clean : 
	-rm *.o 
