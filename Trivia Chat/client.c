#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include "client.h"
#include "socket.h"
#include "ui.h"
#include <poll.h>

// The time for a warning to be displayed
#define WARNING_TIME 5
#define DEFAULT_MAP_SIZE 128

int game_over = 0;

question_t questions_log[TOTAL_QUESTIONS];
int cur_question_num = 0;

typedef struct user {
  char* username;
  int points;
  int lifelines;
  int fd;
} user_t;

void display_leaderboard(int num_users) {
  user_t leaderboard[num_users];
  int question_result;
  read(connection_fd, &question_result, sizeof(int));
  int correct_index;
  read(connection_fd, &correct_index, sizeof(int));

  char * correct_answer = questions_log[cur_question_num].answers[correct_index];
  if(question_result == -1) {
    printf("You answered question %d incorrectly. The correct answer was %s\n", cur_question_num+1, correct_answer);
  } else if(question_result == 1) {
    printf("You answered question %d correctly!\n", cur_question_num+1);
  } else {
    printf("Too slow on question %d! The correct answer was %s\n", cur_question_num+1, correct_answer);
  }

  printf("\n");
  printf("LEADERBOARD after %d Question(s):\n", cur_question_num+1);
  for(int i = 0; i < num_users; i++) {
    int username_len;
    read(connection_fd, &username_len, sizeof(int));
    char temp_username[username_len+1];
    read(connection_fd, temp_username, username_len);
    temp_username[username_len] = '\0';
    leaderboard[i].username = strdup(temp_username);
    read(connection_fd, &leaderboard[i].points, sizeof(int));
  }

  for(int j = 0; j < num_users; j++) {
    printf("%s : %d\n", leaderboard[j].username, leaderboard[j].points);
  }
  printf("\n");
}

void send_message(char message, long time) {

 // Run the callback function provided to ui_init
  if(message == 'q') {
    exit(1);
  } else {
    response_header_t response_header;
    response_header.username_len = username_len;
    response_header.response_type = 'a';

    write(connection_fd, &response_header, sizeof(response_header_t));
    write(connection_fd, username, response_header.username_len);
    write(connection_fd, &cur_question_num, sizeof(int));
    write(connection_fd, &message, sizeof(char));
    write(connection_fd, &time, sizeof(long));
  }
}

void* listen_thread(void* p) {
  
  while(true){
    header_t header;
    int ans_lens[4];

    if(read(connection_fd, &header, sizeof(header_t)) > 0){
      if(header.message_type == 'q') {
        read(connection_fd, &cur_question_num, sizeof(int));
        questions_log[cur_question_num].question_num = cur_question_num;

        int category_len;
        read(connection_fd, &category_len, sizeof(int));

        char category_text[category_len+1];
        read(connection_fd, &category_text, sizeof(char)*category_len);
        category_text[category_len] = '\0';
        questions_log[cur_question_num].category_text = strdup(category_text);
        printf("Category: %s\n", questions_log[cur_question_num].category_text);

        int difficulty_len;
        read(connection_fd, &difficulty_len, sizeof(int));

        char difficulty_text[difficulty_len+1];
        read(connection_fd, &difficulty_text, difficulty_len);
        difficulty_text[difficulty_len] = '\0';
        questions_log[cur_question_num].difficulty_text = strdup(difficulty_text);
        printf("Difficulty: %s\n", questions_log[cur_question_num].difficulty_text);

        int question_len;
        read(connection_fd, &question_len, sizeof(int));

        char question_text[question_len+1];
        read(connection_fd, &question_text, question_len);
        question_text[question_len] = '\0';
        questions_log[cur_question_num].question_text = strdup(question_text);

        printf("Question %d: %s\n",cur_question_num+1, questions_log[cur_question_num].question_text);

        for(int i = 0; i<4; i++) {
            int ans_len;
            read(connection_fd, &ans_len, sizeof(int));
            
            char answer[ans_len+1];
            read(connection_fd, &answer, ans_len);
            answer[ans_len] = '\0';
            questions_log[cur_question_num].answers[i] = strdup(answer);

            printf("Answer %c: %s\n", i+'A', answer);
             
            
          }

        printf("Enter your answer: \n");

        int duration = 20;
        wait_for_input(duration);
      } else if(header.message_type == 'l') {
        display_leaderboard(header.num_users);
      }
    }
  }
}
//TODO: VARIABLE NUMBER OF QUESTIONS
void wait_for_input(int duration) {
  struct timespec start, cur;
  clock_gettime(CLOCK_MONOTONIC, &start);
  clock_gettime(CLOCK_MONOTONIC, &cur);
  int warning_displayed = 0;
  long end_time_s = duration;
  long prev_time = cur.tv_sec;

  while(cur.tv_sec <= start.tv_sec+duration) {
    if(cur.tv_sec > prev_time) {
      prev_time = cur.tv_sec;
    }
    if(warning_displayed == 0 && cur.tv_sec >= start.tv_sec+duration-WARNING_TIME) {
      //Display the warning
      printf("%d second warning\n", WARNING_TIME);
      warning_displayed = 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &cur);
    char* buff = NULL;
    size_t len = 0;
    struct pollfd my_pollfd;
    my_pollfd.fd = STDIN_FILENO;
    my_pollfd.events = POLLIN;
    my_pollfd.revents = 0;
    int poll_result;
    int ch;
    if((poll_result = poll(&my_pollfd, 1, 0)) > 0) {
      ch = fgetc(stdin);
      int nl = fgetc(stdin);
      if(nl != '\n') {
        printf("You entered more than 1 char\n");
      } else {
        send_message(ch, end_time_s-(cur.tv_sec-start.tv_sec));
      }

      return;
    }
  }
  printf("Game: You failed to enter an answer. Be better next time\n\n");
}


int main(int argc, char** argv) {
  // Make sure the arguments include a username
  if(argc != 2 && argc != 4) {
    fprintf(stderr, "Usage: %s <username> [<peer> <port number>]\n", argv[0]);
    exit(1);
  }
  
  // Save the username in a global
  username = argv[1];
  username_len = strlen(username);

  if(argc == 4) {
    // Unpack arguments
    char* peer_hostname = argv[2];
    unsigned short peer_port = atoi(argv[3]);
    
    // Connect to the server
    int socket_fd = socket_connect(peer_hostname, peer_port);
    if(socket_fd == -1) {
      perror("Failed to connect");
      exit(2);
    }
    printf("Welcome to the Trivia Game!\n");
    printf("Please wait for the first question\n\n");
    struct timespec tim;
    tim.tv_sec  = 1L;
    tim.tv_nsec = 0;
    nanosleep(&tim, NULL);
    //Send message to server to give the new username and length of username
    response_header_t response_header;
    response_header.username_len = username_len;
    response_header.response_type = '!';
    write(socket_fd, &response_header, sizeof(response_header_t));
    write(socket_fd, username, response_header.username_len);
    connection_fd = socket_fd;
    //args_t client_args;
    //client_args.id = socket_fd;
    //pthread_create(&connection, NULL, func, (void*)&client_args);
   
    pthread_create(&connection, NULL, listen_thread, NULL);

  }
  while(!game_over) {

  }
}
