CC=gcc                    # Compilador
CFLAGS=-g -Wall						# Deugger + All warnings
LDFLAGS=-lthreadpool      # Bibliotecas que tengo que linkear

all:
	@echo "-----------------------------------------------------------------------"
	@echo " 💡 HELP 💡"
	@echo "-----------------------------------------------------------------------"
	@echo " make tp                          # Compila client + server"
	@echo " make server                      # Compila solo el server"
	@echo " make tarea1                      # Compila la tarea1"
	@echo " make client                      # Compila solo el client"
	@echo " make udp_ping                    # Envia datos al server UDP"
	@echo " make clean                       # Limpia los binarios"
	@echo "-----------------------------------------------------------------------"

tp: clean client alive_client server

server: clean_server threadpool.o
	$(CC) $(CFLAGS) server.c -o server threadpool.o

client: clean_client
	$(CC) $(CFLAGS) client.c -o client

alive_client: clean_alive_client
	$(CC) $(CFLAGS) alive_client.c -o alive_client

# Tarea 1 -> Ver los comentarios adentro de tarea1.c
tarea1: tarea1.c
	$(CC) $(CFLAGS) -lcurses -o tarea1 tarea1.c

threadpool.o: threadpool.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o threadpool.o threadpool.c

clean_server:
	@echo "🧹 Cleaning Server 🧹"
	@rm -f server
	@rm -rf server.DSYM
	@rm -f threadpool.o

clean_client:
	@echo "🧹 Cleaning Client 🧹"
	@rm -rf client
	@rm -rf client.DSYM

clean_alive_client:
	@echo "🧹 Cleaning Alive Client 🧹"
	@rm -rf alive_client
	@rm -rf alive_client.DSYM

clean: clean_server clean_client clean_alive_client

.PHONY: clean
