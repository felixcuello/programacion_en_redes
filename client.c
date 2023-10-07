#include <stdio.h>      // printf
#include <stdlib.h>     // atoi
#include <netinet/in.h> // sockaddr_in
#include <unistd.h>     // close, write
#include <strings.h>    // bzero
#include <arpa/inet.h>  // inet_pton
#include <sys/types.h>  // socket
#include <sys/socket.h> // socket, connect

int main(int argc, char** argv) {
  int sockfd;
  struct sockaddr_in servaddr;

  if (argc != 3) {
    printf("usage: client <server_addres> <port>");
    return 0;
  }

  char* server_address = argv[1];
  int port = atoi(argv[2]);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  inet_pton(AF_INET, server_address, &servaddr.sin_addr);

  if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
    printf("client>> No me pude conectar al server\n");
    return 1;
  }

  printf("client>> Conectado al server\n");
  printf("client>> enviando PING\n");
  
  if(write(sockfd, "PING", 4) != 4) {
    printf("client>> Error al escribir\n");
    return 1;
  }

  char buffer[256];
  bzero(buffer, sizeof(buffer));

  if(read(sockfd, buffer, sizeof(buffer)) < 0) {
    printf("client>> Error al leer\n");
    return 1;
  }

  printf("client>> Leido => %s\n", buffer);

  close(sockfd);

  return 0;
}

/*
      TCP Server                                         TCP Client [telnet]

√ 1   socket()

√ 2   bind()

√ 3   listen()

√ 4   accept()

5                                                      √ socket()

6            <---------------------------------------  √ connect()

7            <---------------------------------------  write()

8   read()

9   write()  --------------------------------------->  read()

10  close()  --------------------------------------->  read()

11                                                     close()
*/
