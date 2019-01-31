#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <sys/time.h>
#include <iostream>

int main (int argc, char* argv[]) {

  int socket_fd, status;
  char *filename, *hostname, *port; 
  struct addrinfo *addr;
  struct addrinfo hin; 
  int one = 1;

 //should only be 3 arguments
 if (argc != 4) {
    fprintf(stderr, "ERROR: Incorrect number of args\n");
    exit(1);
  }

  //port number should not be in 0-1023 range
  if (atoi(port) <= 1023) {
    fprintf(stderr, "ERROR: Invalid port number\n");
    exit(1);
  }
  else {   //parse arguments
  	hostname = argv[1];
  	port = argv[2];
    filename = argv[3];
  }
//addtional arg error checking
  memset(&hin, 0, sizeof(hin));
  hin.ai_family = AF_INET;
  hin.ai_socktype = SOCK_STREAM; 
  status = getaddrinfo(hostname, port, &hin, &addr);
  if (status != 0)
    {
      fprintf(stderr, "ERROR: Argument Error\n");
      exit(1);
    }
//open the socket
  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "ERROR: Could not create socket\n");
    exit(1);
  }

  //make socket reusable
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1)
    {
      fprintf(stderr,"ERROR: Call to setsockopt() failed\n");
      exit(1);
    }
//make socket non-blocking
  if (fcntl(socket_fd, F_SETFL, O_NONBLOCK))
    {
      fprintf(stderr, "ERROR: Unable to get socket to nonblocking\n");
      exit(1);
    }
//timeout stuff
  fd_set active_fd_set;
  struct timeval timeout;
  FD_ZERO(&active_fd_set);
  FD_SET(socket_fd, &active_fd_set);
  timeout.tv_sec = 15; 
  timeout.tv_usec = 0;
  

  int rec = connect(socket_fd, addr->ai_addr, addr->ai_addrlen); 
  if ( rec < 0)
    {
      if(errno == EINPROGRESS)
	{

	  int recsel = select(socket_fd+1, NULL, &active_fd_set, NULL, &timeout);
	  if(recsel < 0)
	    {
	      fprintf(stderr, "ERROR: Unable to select\n");
	      exit(1);
	    }
	  else if( recsel == 0)
	    {
	      fprintf(stderr, "ERROR: Connection timedout\n");
	      exit(1);
	    }
	  else
	    {
	      printf("Connection success.\n");	 //need?     
	    }
	}
      else
	{
	  fprintf(stderr,"ERROR: Unable to connect socket\n");
	  exit(1);
	}
    }
  else
    {
      fprintf(stderr,"ERROR: Unable to connect socket\n");
      exit(1);
    }

  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  if (getsockname(socket_fd, (struct sockaddr *)&client_addr, &client_addr_len) == -1)
    {
      fprintf(stderr,"ERROR: Call to getsockname() failed\n");
      exit(1);
    }
  
  char IP_string[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(client_addr.sin_family, &client_addr.sin_addr, IP_string, sizeof(IP_string));
  std::cout << "Set up a connection from: " << IP_string << ":" <<
    ntohs(client_addr.sin_port) << std::endl;

  FILE* f_pointer= fopen(filename,"rb");
  if(f_pointer == NULL){
      fprintf(stderr, "ERROR: File could not be opened\n");
      exit(1);
    }
  while(1){
      int recsel = select(socket_fd+1, NULL, &active_fd_set, NULL, &timeout);
      if(recsel < 0){
	  fprintf(stderr, "ERROR: Select failed\n");
	  exit(1);
	}
      else if(recsel==0){
	  fprintf(stderr, "ERROR: Connection timeout\n");
	  exit(1);
	}
	//use buffer to read
      char buffer[1024];
      int buffer_size = 1024;
      memset(buffer, '\0', buffer_size);
      int read_byte = fread(buffer,sizeof(char), buffer_size, f_pointer);
      if(read_byte == 0){
	  fprintf(stdout, "No bytes are read\n");
	  fclose(f_pointer);
	  break;
	} 
      else if (read_byte < 0){
	  fprintf(stderr, "ERROR: File read error\n");
	  exit(1);
	}
      else{
	  int byte_sent = send(socket_fd, buffer, read_byte, 0);
	  fprintf(stdout, " %d bytes sent.\n", byte_sent);
	  if(byte_sent < 0)
	    {
	      fprintf(stderr, "ERROR: Unable to send files\n");
	      exit(1);
	    }
	}
    } 
  close(socket_fd);
  freeaddrinfo(addr);
  printf("Program Exited successfully\n");
  exit(0);
}


