CC=gcc                    # Compilador
CFLAGS=-g -Wall						# Deugger + All warnings
LDFLAGS=-lcurses          # Bibliotecas que tengo que linkear

all:
	@echo "-----------------------------------------------------------------------"
	@echo " 💡 HELP 💡"
	@echo "-----------------------------------------------------------------------"
	@echo " make tp                          # Compila client + server"
	@echo " make server                      # Compila solo el server"
	@echo " make client                      # Compila solo el client"
	@echo " make udp_ping                    # Envia datos al server UDP"
	@echo " make clean                       # Limpia los binarios"
	@echo " make shell                       # Ingresa al shell del server"
	@echo " make client_shell                # Ingresa al shell del client"
	@echo " make [SERVICE=service] build     # Construye las imagenes de docker"
	@echo " make [SERVICE=service] up        # Levanta los servicios de docker"
	@echo "-----------------------------------------------------------------------"

tp: clean client server

server: clean_server
	$(CC) $(CFLAGS) server.c -o server

client: clean_client
	$(CC) $(CFLAGS) client.c -o client

# Tarea 1 -> Ver los comentarios adentro de tarea1.c
tarea1: tarea1.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o tarea1 tarea1.c

udp_ping:
	@echo "ping" | nc -u -w 1 127.0.0.1 4321

clean_server:
	@echo "🧹 Cleaning Server 🧹"
	@rm -f server
	@rm -rf server.DSYM

clean_client:
	@echo "🧹 Cleaning Client 🧹"
	@rm -rf client
	@rm -rf client.DSYM

clean: clean_server clean_client

shell:
	@docker compose run --rm --service-ports -ti server /bin/bash

client_shell:
	@docker compose run --rm --service-ports -ti client /bin/bash

build:
	@echo "🚀 Building 🚀"
	@docker compose build --no-cache $(SERVICE)

up:
	@echo "🚀 Starting 🚀"
	@docker compose up --service-ports $(SERVICE)

.PHONY: clean
