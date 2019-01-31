#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>

//TODO: sigquit or sigterm

//gloabals needed
 void* connection_handler(void* param);
 void signal_handler(int signo);
 char *file_directory;
  int connection_num = 0;

int main (int argc, char* argv[]) {
  int socket_fd, status;
  char* port;
  int one = 1;
//should have 3 args
  if (argc != 3) {
    fprintf(stderr, "ERROR: Incorrect number of args\n");
    exit(1);
  }
  else {  //parse arguments
  	port = argv[1];
  	file_directory = argv[2];
  }
  //port number should not be between 0 and 1023
  if (atoi(port) <= 1023) {
    fprintf(stderr, "ERROR: Invalid port number\n");
    exit(1);
  }
  //signal handlers
  signal(SIGQUIT, signal_handler);
  signal(SIGTERM, signal_handler);

  struct addrinfo hin;
  struct addrinfo *addr;  
  memset(&hin, 0, sizeof hin); 
  hin.ai_family = AF_INET;     
  hin.ai_socktype = SOCK_STREAM; 
  hin.ai_flags = AI_PASSIVE; //have IP filled
  if ((status = getaddrinfo(NULL, port , &hin, &addr)) != 0){
      fprintf(stderr, "ERROR: %s", gai_strerror(status));
      exit(1);
    }

  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "ERROR: Could not create socket\n");
    exit(1);
  }

  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1){
      fprintf(stderr,"ERROR: call to setsockopt() failed\n");
      exit(1);
    }

  if (bind(socket_fd, addr->ai_addr, addr-> ai_addrlen) == -1) {
      fprintf(stderr, "ERROR: Unable to bind socket to address\n");
    	exit(1);
  }
    if (listen(socket_fd, 10) == -1) {
      fprintf(stderr, "ERROR: Socket unable to listen\n");
    	exit(1);
  }
while(true){
      struct sockaddr_in client_addr;
      socklen_t client_addr_size = sizeof(client_addr);
      int client_socket_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &client_addr_size);
      if (client_socket_fd == -1){
	      fprintf(stderr, "ERROR: Socket accept failed\n");
	      exit(1);   
	}
      else {
	  connection_num++;
	  char ipstr[INET_ADDRSTRLEN] = {'\0'};
	  inet_ntop(client_addr.sin_family, &client_addr.sin_addr, ipstr, sizeof(ipstr));
	  std::cout << "Accept a connection from: " << ipstr << ":" <<
	    ntohs(client_addr.sin_port) << std::endl;
	  
	  pthread_t thread_id;
	  //create thread and send to connection handler
	  if(pthread_create(&thread_id, NULL, connection_handler, (void*)&client_socket_fd)){
	      fprintf(stderr, "ERROR: pthread creation error\n");
	      exit(1);
	    }
	  pthread_detach(thread_id);
	}	
    }
  freeaddrinfo(addr);
  exit(0);
}
void signal_handler(int signo)
{
  if (signo == SIGQUIT || signo == SIGTERM)
    exit(0);
}

void* connection_handler(void* param)
{
  char f_path[1000];
  sprintf(f_path,"%s/%d.file", file_directory, connection_num);
  printf("Enters connection handler\n");
  int client_socket_fd = *(int*)param;
  if (fcntl(client_socket_fd, F_SETFL, O_NONBLOCK))
    {
      fprintf(stderr, "ERROR: set nonblocking error\n");
      exit(1);
      }
  fd_set active_fd_set;
  FD_ZERO(&active_fd_set);
  FD_SET(client_socket_fd, &active_fd_set);
  struct timeval timeout;
  timeout.tv_sec = 15; 
  timeout.tv_usec = 0;
  FILE* f_pointer = fopen(f_path, "wb");
  printf("File %s created\n", f_path);
  if(f_pointer == NULL)
    {
      fprintf(stderr, "ERROR: create file failed\n");
      exit(1);
    }
  while(1)
    {
      int recsel = select(client_socket_fd+1, &active_fd_set, NULL, NULL, &timeout);
	if(recsel < 0)
	  {
	    fprintf(stderr, "ERROR: Select failed\n");
	    exit(1);
	  }
	else if( recsel == 0){
	    close(client_socket_fd);
	    fprintf(stderr, "ERROR: Connection timeout\n");
	    fclose(f_pointer);
	    f_pointer = fopen(f_path, "wb");
	    printf("Reopen\n");
	    if(f_pointer == NULL){
		fprintf(stderr, "ERROR: file reopen error\n");
		exit(1);
	      }
	    char message[20] = "ERROR";
	    fflush(f_pointer);
	    int written = fwrite(message, sizeof(char), 6, f_pointer);
	    printf("%d written\n", written);
	    if( written < 0){
		fprintf(stderr, "ERROR: Write error failed\n");
		exit(1);
	      }
	    fclose(f_pointer);
	    return 0;
	  }

	char buffer[1024];
	int buffer_size = 1024;
	memset(buffer, '\0', buffer_size);
	int byte_received = recv(client_socket_fd, buffer, buffer_size, 0);
	printf("%d bytes received\n", byte_received);
	if(byte_received < 0){
	    fprintf(stderr, "ERROR: Receiving data failed\n");
	    exit(1);
	  }
	else if(byte_received == 0){ 
	    printf("Nothing recieved\n");
	    fclose(f_pointer);
	    break;
	  }
	else{
	    printf("Received %d bytes\n", byte_received);
	    if(fwrite(buffer, sizeof(char), byte_received, f_pointer) ==  0){
		fprintf(stderr, "ERROR: Write to file failed\n");
		exit(1);
	      }
	  }
	    	
    }
  return 0; 
  
}