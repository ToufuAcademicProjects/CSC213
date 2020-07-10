#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "socket.h"
#include "ui.h"
#define ded -1

// Keep the username in a global so we can access it from the callback
const char* username;
int connections[1000];
int cur_connection;
pthread_mutex_t lock;

typedef struct header
{
  int username_length;
  int length;
} header_t;

typedef struct args
{
  int client_socket_fd;
  int connection_number;
} args_t;


// This function is run whenever the user hits enter after typing a message
void input_callback(const char* message) {
  if(strcmp(message, ":quit") == 0 || strcmp(message, ":q") == 0) {
    ui_exit();
  } else {

    int user_len = strlen(username)+1;
    int mes_len = strlen(message)+1;
    
    
    //Create the header
    header_t head;
    head.username_length = user_len;
    head.length = mes_len;
    
    int i = 0;
    //Send the message to all the peers, once
    while(i<cur_connection)
      {
        if(write(connections[i], &head, sizeof(header_t))<=0)
          {
            //Remove the peer from the data structure
            pthread_mutex_lock(&lock);
            connections[i] = ded;
            pthread_mutex_unlock(&lock);

          }
        //ui_display("We are", "stuff");

        if(write(connections[i], username, user_len*(sizeof(char)))<=0)
          {
            pthread_mutex_lock(&lock);
            connections[i] = ded;
            pthread_mutex_unlock(&lock);


          }
        if(write(connections[i], message, mes_len*sizeof(char))<=0)
          {
            pthread_mutex_lock(&lock);
            connections[i] = ded;
            pthread_mutex_unlock(&lock);

          }
        i++;
      }
    ui_display(username, message);
  }
}

void* communicate(void* arguments)
{
  args_t* input = (args_t*)arguments;
  int socket_fd = input->client_socket_fd;
  int connection_num = input->connection_number;
  while(connections[connection_num]!= ded)
    {
      header_t buffer;
      //We only read the length of the username and length of the message here

      ssize_t file_header = read(socket_fd, &buffer, sizeof(header_t));
      if(file_header == -1)
        {
          pthread_mutex_lock(&lock);
          connections[connection_num] = ded;
          pthread_mutex_unlock(&lock);

        }

      int user_length = buffer.username_length;
      int msg_length = buffer.length;
      
      //We read in the actual username here
      char user_name[user_length];

      if(read(socket_fd, user_name, user_length )<=0)
        {
          pthread_mutex_lock(&lock);
          connections[connection_num] = ded;
          pthread_mutex_unlock(&lock);

        }

      char message[msg_length];
      if(read(socket_fd, message, msg_length)<=0)
        {
          pthread_mutex_lock(&lock);
          connections[connection_num] = ded;
          pthread_mutex_unlock(&lock);

        }
      if(connections[connection_num] != ded) 
        ui_display(user_name, message);
      
      //Write the message to our peers
      int i = 0;
      while(i<cur_connection && connections[connection_num] != ded)
        {
          if(connections[i] != ded && connections[i] != socket_fd)
            {
              if(write(connections[i], &buffer, sizeof(header_t))<=0)
                {
                  pthread_mutex_lock(&lock);
                  connections[i] = ded;
                  pthread_mutex_unlock(&lock);

                }
              if(write(connections[i], user_name, user_length)<=0)
                {
                  pthread_mutex_lock(&lock);
                  connections[i] = ded;
                  pthread_mutex_unlock(&lock);

                }
              if(write(connections[i], message, msg_length)<=0)
                {
                  pthread_mutex_lock(&lock);
                  connections[i] = ded;
                  pthread_mutex_unlock(&lock);

                }
            }
          i++;
        }
    }
  
 
  return NULL;
}
void* open_server(void * server_socket_fd)
{
  int client_socket_fd;

 
  while(1==1)
    {
      // Wait for a client to connect
      client_socket_fd = server_socket_accept((int)server_socket_fd);
      //ui_display("INFO", "connecting user");
      if(client_socket_fd == -1) {
        perror("accept failed");
        exit(2);
      }
      ui_display("INFO", "User connected");
      pthread_mutex_lock(&lock);

      args_t* input= malloc(sizeof(args_t));
      input->client_socket_fd = client_socket_fd;
      input->connection_number = cur_connection;

      connections[cur_connection] = client_socket_fd;
      cur_connection++;
      
      pthread_mutex_unlock(&lock);

      pthread_t thread;
      pthread_create(&thread, NULL, communicate, input);

    }
  

  return NULL;
}


int main(int argc, char** argv) {
  pthread_mutex_init(&lock, NULL);

  
  // Make sure the arguments include a username
  if(argc != 2 && argc != 4) {
    fprintf(stderr, "Usage: %s <username> [<peer> <port number>]\n", argv[0]);
    exit(1);
  }
  cur_connection = 0;
  // Save the username in a global
  username = argv[1];
  
  
  // Did the user specify a peer we should connect to?
  if(argc == 4) {
    // Unpack arguments
    char* peer_hostname = argv[2];
    unsigned short peer_port = atoi(argv[3]);
    // Connect to the server
    int socket_fd = socket_connect(peer_hostname, peer_port);   //Here is the issue, socket is 3, which means we're reading from 3?
    if(socket_fd == -1) {
      perror("Failed to connect");
      exit(2);
    }
    ui_display("INFO", "We have connected to somewhere");
    //int server_socket_fd = server_socket_accept(socket_fd);
    //Put our connection into an array
    pthread_mutex_lock(&lock);

    connections[cur_connection] = socket_fd;
    cur_connection++;
    //printf("Our socket-fd was %d\n", socket_fd);

    args_t* input= malloc(sizeof(args_t));
    input->client_socket_fd = socket_fd;
    input->connection_number = cur_connection-1;
    pthread_mutex_unlock(&lock);

    pthread_t thread;
    pthread_create(&thread, NULL, communicate, input);
  }
 
  // Open a server socket
  unsigned short port = 0;
  int server_socket_fd = server_socket_open(&port);
  if(server_socket_fd == -1) {
    perror("Server socket was not opened");
    exit(2);
  }

  //Start listening
  if(listen(server_socket_fd, 1)) {
    perror("listen failed");
    exit(2);
  }

  

  pthread_t thread;
  pthread_create(&thread, NULL, open_server, (void*)(intptr_t)server_socket_fd);
  
  
  // Set up the user interface. The input_callback function will be called
  // each time the user hits enter to send a message.
  ui_init(input_callback);

  char buffer [19];
  snprintf ( buffer, 19, "The port is %d", port );
  ui_display("INFO", buffer);
  // Once the UI is running, you can use it to display log messages
  //ui_display("INFO", "This is a handy log message.");
  
  // Run the UI loop. This function only returns once we call ui_stop() somewhere in the program.
  ui_run();
  
  return 0;
}
