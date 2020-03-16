############################# Makefile ##########################
all: dkt
dkt: tcp_server.o udp_server.o udp_client.o tcp_client.o main.o
	gcc -o dkt tcp_server.o udp_server.o udp_client.o tcp_client.o main.o
tcp_server.o: tcp_server.c server.h
	gcc -o tcp_server.o -c tcp_server.c
udp_server.o: udp_server.c server.h
	gcc -o udp_server.o -c udp_server.c
tcp_client.o: tcp_client.c server.h
	gcc -o tcp_client.o -c tcp_client.c
udp_client.o: udp_client.c server.h
	gcc -o udp_client.o -c udp_client.c
main.o: main.c server.h
	gcc -o main.o -c main.c
clean:
	rm -rf *.o
mrproper: clean
	rm -rf dkt
