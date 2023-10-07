CC=gcc                    # Compilador
CFLAGS=-g -Wall						# Deugger + All warnings
LDFLAGS=-lcurses          # Bibliotecas que tengo que linkear

OBJS = main.o # some_library.o

tp: client server

server:
	$(CC) $(CFLAGS) server.c -o server

client:
	$(CC) $(CFLAGS) client.c -o client

# Tarea 1 -> Ver los comentarios adentro de tarea1.c
tarea1: tarea1.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o tarea1 tarea1.c

clean:
	rm -f $(OBJS) $(TARGET)    # Programa original
	rm -rf tarea1 *.dSYM       # Tarea1
	rm -f server client        # Tarea2

shell:
	docker compose run -ti tp_server /bin/bash

client_shell:
	docker compose run -ti tp_client /bin/bash

build:
	docker compose build

.PHONY: clean
