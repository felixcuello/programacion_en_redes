#include <stdio.h>       // printf
#include <stdlib.h>      // atoi
#include <sys/socket.h>  // socket, bind, listen, accept
#include <sys/syscall.h> // __NR_gettid
#include <netinet/in.h>  // sockaddr_in
#include <unistd.h>      // write
#include <string.h>      // strncmp
#include <pthread.h>     // pthread_creat
#include <sys/types.h>
#include <fcntl.h>       // fcntl
#include <netdb.h>

#define LISTEN_QUEUE_SIZE 2

int main(int argc, char** argv) {
  if(argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return 1;
  }

  int port = atoi(argv[1]);

  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
  fcntl(server_socket, F_SETFL, O_NONBLOCK);

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

  fd_set readfds;
  char buffer[256];

  FD_ZERO(&readfds);
  FD_SET(server_socket, &readfds);
  int max_fd = server_socket;
  struct timeval tv;
  tv.tv_sec = 2;
  tv.tv_usec = 0;

  while(1) {
    int n_fds = select(max_fd + 1, &readfds, NULL, NULL, &tv);

    printf("server >> max_fd=%d\n", max_fd);
    printf("server >> n_fds=%d\n", n_fds);

    if(n_fds < 0) {
      printf("Error: select()\n");
      return 1;
    }

    if(FD_ISSET(server_socket, &readfds)) {
      struct sockaddr_in client_addr;
      socklen_t client_addr_len = sizeof(client_addr);

      int incoming_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);

      if(incoming_socket < 0) {
        printf("Error: accept()\n");
        return 1;
      }

      FD_SET(incoming_socket, &readfds);
      max_fd = incoming_socket > max_fd ? incoming_socket : max_fd;
    } else {
      for(int i = 0; i <= max_fd; i++) {
        if(FD_ISSET(i, &readfds)) {
          bzero(buffer, sizeof(buffer));

          int bytes_read = recv(i, (void*)&buffer, sizeof(buffer) - 1, 0);
          if(bytes_read == 0) {
            FD_CLR(i, &readfds);
            max_fd = max_fd == i ? max_fd - 1 : max_fd;

            break;
          } else if(bytes_read < 0) {
            printf("error haciendo recv()\n");
            break;
          }
          printf("server received >> %s\n", buffer);

          // comparar buffer con PING y devolver PONG
          if(strncmp(buffer, "PING", 4) == 0) {
            printf("server returns >> PONG\n");
            write(i, "PONG\n", 5);
          }
        }
      }
    }
  }

  // Aca no va a llegar nunca
  close(server_socket);
  printf("server>> Server finalizado\n");

  return 0;
}

/*

1    socket()
        |
2    bind()
        |
3    listen()
        |
4    select()
5       |     \
        |      \
        |       \
6    accept()    recv()
        |         |    \
        |         |     send()
        |         |       \
     go to 4   close()    go to 4

*/
