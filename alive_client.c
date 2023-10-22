#include <arpa/inet.h>  // inet_pton
#include <fcntl.h>      // O_NONBLOCK
#include <netinet/in.h> // sockaddr_in
#include <stdio.h>      // printf
#include <stdlib.h>     // atoi
#include <strings.h>    // bzero
#include <sys/socket.h> // socket, connect
#include <sys/types.h>  // socket
#include <unistd.h>     // close, write

#define TRUE                1
#define BUFFER_SIZE       256
#define RETRY_SLEEP_TIME    1
#define MISSING_THRESHOLD   5

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("usage: %s <server_addres> <port>", argv[0]);
    return 0;
  }

  int missing_count = 0;
  char* server_address = argv[1];
  int port = atoi(argv[2]);

  while(TRUE) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, server_address, &(servaddr.sin_addr));

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)); // Set timeout if the server does not respond

    if(sendto(fd, "ping\n", 5, 0, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
      printf("Error: sendto()\n");
      missing_count++;

      sleep(RETRY_SLEEP_TIME);
      close(fd);
      continue;
    }

    if(missing_count > MISSING_THRESHOLD) {
      printf("OH OH... el server se fue a disney! :-<  \n");
      close(fd);
      return 1;
    }

    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    socklen_t addr_size = sizeof(servaddr);
    recvfrom(fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&servaddr, &addr_size);

    if(strncmp(buffer, "ALIVE", 5) == 0) {
      printf("El servidor está VIVO!!!\n");
      missing_count = 0;
    } else {
      printf("El servidor NO RESPONDE :( ( Retry %d / %d ) !!!\n", missing_count, MISSING_THRESHOLD);
      missing_count++;
    }

    close(fd);
    sleep(RETRY_SLEEP_TIME);
  }
  return 0;
}
