############################# Makefile ##########################
all: dkt
dkt: tcp.o udp_server.o udp_client.o main.o
	gcc -o dkt tcp.o udp_server.o udp_client.o main.o
udp_server.o: udp_server.c server.h
	gcc -o udp_server.o -std=c11 -Wall udp_server.c
tcp.o: tcp.c server.h
	gcc -o tcp.o -c -std=c11 -Wall tcp.c
udp_client.o: udp_client.c server.h
	gcc -o udp_client.o -c udp_client.c
main.o: main.c server.h
	gcc -o main.o -std=c11 -Wall main.c
clean:
	rm -rf *.o
mrproper: clean
	rm -rf dkt
