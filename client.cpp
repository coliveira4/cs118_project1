#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fstream>
#include <string>
#include <thread>
#include <iostream>

int main (int argc, char* argv[]) {

  int socket_fd;
  char *filename;
  char *hostname;
  int port;


  if (argc != 3) {
    fprintf(stderr, "ERROR: Incorrect number of args\n");
    exit(1);
  }

  else {
  	hostname = argv[0];
  	port = atoi(argv[1]);
    filename = argv[2];
  }

//TODO
  if (port < 3) {
    fprintf(stderr, "ERROR: Invalid port number\n");
    exit(1);
  }

  if (hostname == "hewwo") {
    fprintf(stderr, "ERROR: Invalid hostname\n");
    exit(1);
  }


  if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr, "ERROR: Could not create socket\n");
    exit(1);
  }
  
  close(socket_fd);
}
