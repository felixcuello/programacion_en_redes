#include <stdio.h>      // printf
#include <stdlib.h>     // atoi
#include <sys/socket.h> // socket, bind, listen, accept
#include <netinet/in.h> // sockaddr_in
#include <unistd.h>     // write
#include <string.h>     // strncmp
#include <pthread.h>    // pthread_creat

#define LISTEN_QUEUE_SIZE 1

void atender_cliente(int* socket) {
  int client_socket = *socket;
  //pthread_t thread_id = pthread_self();

  printf("server>> Cliente conectado (socket: %d)\n", client_socket);
  char buffer[256];
  bzero(buffer, sizeof(buffer));
  while(1) {
    int bytes_read = read(client_socket, buffer, sizeof(buffer));

    if(bytes_read < 0) {
      printf("Error!\n");
      break;
    }

    if(bytes_read == 0) {
      printf("server>> Cliente desconectado\n");
      break;
    }

    // comparar buffer con PING y devolver PONG
    if(strncmp(buffer, "PING", 4) == 0) {
      printf("server >> enviando PONG\n");
      write(client_socket, "PONG\n", 5);
    }

    // comparar buffer con QUIT y cerrar el socket
    if(strncmp(buffer, "QUIT", 4) == 0) {
      write(client_socket, "BYEBYE\n", 7);
      close(client_socket); // no chequear errores, porque se esta yendo
      break;
    }
  }
}

int main(int argc, char** argv) {
  if(argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return 1;
  }

  int port = atoi(argv[1]);

  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(server_socket < 0) {
    printf("Error: socket()\n");
    return 1;
  }

  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    printf("Error: bind()\n");
    return 1;
  }

  if(listen(server_socket, LISTEN_QUEUE_SIZE) < 0) {
    printf("Error: listen()\n");
    return 1;
  }
  printf("server>> Server iniciado en el puerto %d\n", port);

  while(1) {
    printf("server>> Esperando cliente\n");
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);

    if(client_socket < 0) {
      printf("Error: accept()\n");
      return 1;
    }

    pthread_t thread = NULL;
    pthread_create(&thread, NULL, (void*)atender_cliente, (void*)&client_socket);
  }

  // Aca no va a llegar nunca
  close(server_socket);
  printf("server>> Server finalizado\n");

  return 0;
}

/*
      TCP Server                                         TCP Client [telnet]

√ 1   socket()

√ 2   bind()

√ 3   listen()

√ 4   accept()

5                                                      socket()

6            <---------------------------------------  connect()

7            <---------------------------------------  write()

8   read()

9   write()  --------------------------------------->  read()

10  close()  --------------------------------------->  read()

11                                                     close()
*/
