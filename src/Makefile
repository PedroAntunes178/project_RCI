############################# Makefile ##########################
all: dkt
dkt: tcp.o udp.o main.o
	gcc -o dkt tcp.o udp.o main.o
udp_server.o: udp.c server.h
	gcc -o udp.o -Wall -c udp.c
tcp.o: tcp.c server.h
	gcc -o tcp.o -Wall -c tcp.c
main.o: main.c server.h
	gcc -o main.o -Wall -c main.c
clean:
	rm -rf *.o
mrproper: clean
	rm -rf dkt
