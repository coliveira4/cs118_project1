#include <string>
#include <thread>
#include <iostream>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main (int argc, char* argv[]) {

  int socket_fd;
  char *file_directory;
  int port;

  if (argc != 2) {
    fprintf(stderr, "ERROR: Incorrect number of args\n");
    exit(1);
  }

  else {
  	port = atoi(argv[0]);
  	file_directory = argv[1];
  }

//TODO
  if (port < 3) {
    fprintf(stderr, "ERROR: Invalid port number\n");
    exit(1);
  }

  if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr, "ERROR: Could not create socket\n");
    exit(1);
  }
  
  close(socket_fd);
}

