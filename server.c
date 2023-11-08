#include <arpa/inet.h>  // inet_pton
#include <fcntl.h>      // O_NONBLOCK
#include <netinet/in.h> // sockaddr_in
#include <pthread.h>    // pthread_create
#include <stdio.h>      // printf
#include <stdlib.h>     // atoi
#include <string.h>     // strncmp
#include <sys/socket.h> // socket, bind, listen, accept
#include <unistd.h>     // write
#include <poll.h>       // poll


//   Algunas constantes para evitar hardcodear valores en el código
// ----------------------------------------------------------------------------
#define TRUE                       1   // Esto es para que quede más lindo cosas como while(TRUE)
#define LISTEN_QUEUE_SIZE          2   // Maxima cola de clientes que vamos a aceptar
#define MAX_CONNECTIONS          100   // Cantidad máxima de conexiones que vamos a aceptar
#define READ_BUFFER_SIZE        1024   // Tamaño del buffer de lectura
#define RESPONSE_BUFFER_SIZE  100000   // Tamaño del buffer de respuesta (1MB)
#define HEARTBEAT_UDP_PORT      4321   // Puerto usado para el heartbeat


//   Forward declarations
// ----------------------------------------------------------------------------
int protocolo_http(int socket);
int aceptar_conexion(int socket); // Acepta una nueva conexión y devuelve el nuevo FD
void udp_heartbeat();
long leer_archivo(char *filename, char **file_content);

//   Main server code
// ----------------------------------------------------------------------------
int main(int argc, char** argv) {
  if(argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return 1;
  }

  pthread_t thread = NULL;
  pthread_create(&thread, NULL, (void *)udp_heartbeat, NULL);

  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(server_socket < 0) {
    printf("Error: socket()\n");
    return 1;
  }

  int port = atoi(argv[1]);

  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, 1);
  fcntl(server_socket, F_SETFL, O_NONBLOCK);                           // Non blocking

  // get socket options
  unsigned int bufferSize = 0;
  unsigned int bufferSizeLenght = sizeof(bufferSize);

  getsockopt(server_socket, SOL_SOCKET, SO_RCVBUF, &bufferSize, &bufferSizeLenght);
  printf("server>> recv buffer size: %u\n", bufferSize);

  getsockopt(server_socket, SOL_SOCKET, SO_SNDBUF, &bufferSize, &bufferSizeLenght);
  printf("server>> send buffer size: %u\n", bufferSize);

  getsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &bufferSize, &bufferSizeLenght);
  printf("server>> reuseaddr: %u\n", bufferSize);


  if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    printf("Error: bind() . ¿Ya se está usando la dirección IP?\n");
    return 1;
  }

  if(listen(server_socket, LISTEN_QUEUE_SIZE) < 0) {
    printf("Error: listen()\n");
    return 1;
  }

  printf("========================================================\n");
  printf("  SERVER INICIADO (escuchando en el puerto: %d)\n", port);
  printf("========================================================\n");

  struct pollfd fds[MAX_CONNECTIONS];   // Array de file descriptors
  int nfds = 1;                         // Número de file descriptors
  memset(fds, 0, sizeof(fds));          // Inicializamos todo el array en 0

  fds[0].fd = server_socket;            // Agrego el server socket a la lista
  fds[0].events = POLLIN;               // Sólo me interesan los eventos de lectura

  int timeout_ms = 5000;                // El timeout del poll es de 1 segundo

  while(TRUE) {
    printf("Cantidad de File Descriptors activos = %d\n", nfds);

    int poll_count = poll(fds, nfds, timeout_ms);

    // De la documentación del man
    // poll() returns the number of descriptors that are ready for I/O, or -1 if an error occurred
    if(poll_count < 0) {
      printf("server>> ERROR en el poll()\n");
      return 1;
    }

    // Arranco de 3, para no monitorear STDIN, STDOUT o STDERR
    for(int candidate_fd=0; candidate_fd < nfds; candidate_fd++) {
      if(fds[candidate_fd].revents == 0) continue; // No hay eventos para este file descriptor

      printf("server>> Evento en el file descriptor %d\n", fds[candidate_fd].fd);

      if(fds[candidate_fd].revents != POLLIN) {
        printf("server>> ERROR, Me llegó un evento que no era de POLLIN %d\n", fds[candidate_fd].revents);
        continue;
      }

      if(fds[candidate_fd].fd == server_socket) {
        int new_fd = aceptar_conexion(server_socket);
        fds[nfds].fd = new_fd;
        fds[nfds].events = POLLIN;
        nfds++;
      } else {
        if(protocolo_http(fds[candidate_fd].fd) < 0)
          printf("server>> Cliente desconectado\n");

        close(fds[candidate_fd].fd);

        fds[candidate_fd].fd = -1;
        fds[candidate_fd].events = 0;
        fds[candidate_fd].revents = 0;
      }

      for(int i=0; i<nfds; i++) {
        if(fds[i].fd == -1) {
          for(int j=i; j<nfds-1; j++) {
            fds[j].fd = fds[j+1].fd;
            fds[j].events = fds[j+1].events;
            fds[j].revents = fds[j+1].revents;
          }
          // i--; // Revisar si no debería dejar esto
          nfds--;
        }
      }
    }
  }

  // Aca no va a llegar nunca, pero lo dejo porque en realidad el while(TRUE) debería
  // ser un while(running) y con los errores fatales debería setear running en FALSE
  close(server_socket);
  printf("server>> Server finalizado\n");

  return 0;
}


//-----------------------------------------------------------------------------
//  Helper functions
//-----------------------------------------------------------------------------


//   Acepta una nueva conexión y devuelve el FD
// ----------------------------------------------------------------------------
int aceptar_conexion(int socket) {
  struct sockaddr client_addr;
  socklen_t client_addr_len;

  return accept(socket, &client_addr, &client_addr_len);
}

//   Atiende al cliente
//   Implementa el protocolo de comunicación
// ----------------------------------------------------------------------------
int protocolo_http(int socket) {
  char buffer[READ_BUFFER_SIZE];
  bzero(buffer, READ_BUFFER_SIZE);

  ssize_t bytes_read = read(socket, &buffer, READ_BUFFER_SIZE);
  if(bytes_read <= 0) return -1;

  printf("server>> Recibido: %s", buffer);  // Este no lleva "\n" al final porque tenemos el \n en el buffer

  //  El cliente envió un GET
  if(strncmp(buffer, "GET", 3) == 0) {
    printf("server >> recibido GET ");
    if(strncmp(buffer, "GET / ", 6) == 0) {
      printf("/\n");

      char* response = "HTTP/1.1 200 OK\nServer: Moncholate\nContent-Type: text/plain\nConnection: close\n\nHello there!\n";

      write(socket, response, strlen(response));
    } else if(strncmp(buffer, "GET /image.jpg", 14) == 0) {
      printf("/image.jpg\n");
      char response_buffer[RESPONSE_BUFFER_SIZE];
      bzero(response_buffer, RESPONSE_BUFFER_SIZE);

      char* file_buffer = NULL;
      long file_size = leer_archivo("image.jpg", &file_buffer);
      sprintf(response_buffer, "HTTP/1.1 200 OK\n"
                               "Server: Moncholate\n"
                               "Content-Type: image/jpeg\n"
                               "Content-Length: %ld\n"
                               "Connection: close\n\n", file_size);

      // TODO: En realidad esto lo hice sin ver el ejemplo. Según lo que vi habría que hacer
      //       "sendfile" , para no tener que tener un buffer del tamaño de la imagen
      write(socket, response_buffer, strlen(response_buffer)); // devuelve los headers
      write(socket, file_buffer, file_size);                   // devuelve la imagen

      free(file_buffer);                                       // liberamos la memoria para evitar memory leaks
    } else if(strncmp(buffer, "GET /favicon.ico", 16) == 0) {
      printf("/favicon.ico\n");
      char response_buffer[RESPONSE_BUFFER_SIZE];
      bzero(response_buffer, RESPONSE_BUFFER_SIZE);

      char* file_buffer = NULL;
      long file_size = leer_archivo("favicon.ico", &file_buffer);
      sprintf(response_buffer, "HTTP/1.1 200 OK\n"
                               "Server: Moncholate\n"
                               "Content-Type: image/vnd\n"
                               "Content-Length: %ld\n\n", file_size);

      write(socket, response_buffer, strlen(response_buffer)); // devuelve los headers
      write(socket, file_buffer, file_size);                   // devuelve la imagen

      free(file_buffer);                                       // liberamos la memoria para evitar memory leaks
    } else {
      // NO pidas otra cosa porque no hay
      char* response = "HTTP/1.1 404 NOT FOUND\nServer: Moncholate\nContent-Type: text/plain\n\n";
      write(socket, response, strlen(response));
    }
  }

  return 0;
}

//   Arranca un thread que responde a los pings en e puerto 4321
// ----------------------------------------------------------------------------
void udp_heartbeat() {
  int udp_server_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(udp_server_fd < 0) {
    printf("server_udp_heatbeat>> ERROR, no pude crear el socket para el heartbeat\n");
    exit(-1);
  }

  struct sockaddr_in udp_server_addr;
  bzero(&udp_server_addr, sizeof(udp_server_addr));

  udp_server_addr.sin_family = AF_INET;
  udp_server_addr.sin_port = htons(HEARTBEAT_UDP_PORT);
  udp_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(udp_server_fd, (struct sockaddr*)&udp_server_addr, sizeof(udp_server_addr)) < 0) {
    printf("server_udp_heartbeat>> ERROR, no pude asignar la dirección. ¿Ya está siendo usada la dirección IP?\n");
    exit(-2);
  }

  printf("server_udp_heatbeat>> INICIADO\n");

  while(TRUE) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[READ_BUFFER_SIZE];

    ssize_t bytes_read = recvfrom(udp_server_fd,                     // File descriptor del socket del server
                                  buffer,                            // Buffer de lectura
                                  READ_BUFFER_SIZE,                  // Tamaño del buffer de lectura
                                  0,                                 // Estos son flags (pero no los usamos)
                                  (struct sockaddr*)&client_addr,    // Datos del cliente
                                  &client_addr_len);                 // Tamaño de la estructura que almacena los datos del cliente
                                                                     //
    printf("server_udp_heartbeat>> Recibido %s", buffer);

    if(bytes_read <= 0) {
      printf("server_udp_heatbeat>> no hubo nada para leer\n");
      continue; // No hubo nada para leer
    }

    printf("server_udp_heartbeat>> Sending ALIVE\n");

    sendto(udp_server_fd, "ALIVE\n", 6, 0, (struct sockaddr*)&client_addr, client_addr_len);
  }
}

//  Lee un archivo y lo guarda en file_content y devuelve el file_size
//
//  WARNING:
//  Crear memoria en una función no es una buena práctica porque le delegamos
//  la responsabilidad de liberar la memoria a quien haya llamado esta función.
//
long leer_archivo(char* file_name, char** file_content) {
  FILE *fp = fopen(file_name, "rb");
  if(fp == NULL) {
    printf("server>> ERROR, no pude abrir el archivo %s\n", file_name);
    exit(-1);
  }

  fseek(fp, 0, SEEK_END);       // seteo el puntero al final
  long file_size = ftell(fp);   // traigo la posición del puntero (tamaño del file)
  fseek(fp, 0, SEEK_SET);       // vuelvo el puntero para adelante

  *file_content = malloc(file_size);
  if (*file_content == NULL) {
    printf("server>> ERROR, no pude reservar la memoria suficiente para el archivo\n");
    exit(-2);
  }

  fread(*file_content, 1, file_size, fp);
  fclose(fp);

  return file_size;
}
