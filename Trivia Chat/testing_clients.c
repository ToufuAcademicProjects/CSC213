#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <jansson.h>
#include <time.h>
#include <pthread.h>


int main(int argc, char** argv) {
	// Make sure the arguments include a username
  if(argc != 2 && argc != 4) {
    fprintf(stderr, "Usage: %s <numClients> <hostname> <port>\n", argv[0]);
    exit(1);
  }
 	int num_clients = atoi(argv[1]);
  char* hostname = strdup(argv[2]);
  char* port = argv[3];
  for(int i = 0; i < num_clients; i++) {
  	pid_t child = fork();
    if(child == 0) {
      printf("i: %d\n", i--);
      char client_name[255];
	  	sprintf(client_name, "%d", i);
	  	char* args[] = { "./client", client_name, hostname, port, NULL };
	  	printf("Creating client with username %s, attempting to connect to host %s on port %s\n", client_name, hostname, port);
	  	if(execvp("./client", args) == -1) {
	  		perror("execvp");
	      printf("\nfailed connection\n");
	      return -1;
	    }
      exit(0);
    }
  	struct timespec timeout;
    timeout.tv_sec  = 1L;
    timeout.tv_nsec = 0;
    nanosleep(&timeout, NULL);
  }

}