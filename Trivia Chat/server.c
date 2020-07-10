#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <jansson.h>
#include <time.h>
#include <pthread.h>

#include "server.h"
#include "socket.h"
#include "ui.h"

//Sends out the leaderboard data to the clients
void report_standing(users_map_t * usermap, int end_game, int question_num) {
  //Creates our temp leaderboard and zeros it out
  user_t* leaderboard[DEFAULT_MAP_SIZE] = {0};
  int index = 0;
  //Adds users into our temp leaderboard
  for(int i = 0; i < usermap->map_size; i++) {
    if(usermap->users_arr[i] != 0) {
      leaderboard[index] = usermap->users_arr[i];
      index++;
    }
  }
  //Sorts the data using bubblesort  
  int notsorted = 1;
  while(notsorted) {
    notsorted = 0;
    for(int i = 0; i < usermap->num_users-1; i++) {
      if(leaderboard[i]->points < leaderboard[i+1]->points) {
        user_t * temp = leaderboard[i];
        leaderboard[i] = leaderboard[i+1];
        leaderboard[i+1] = temp;
        notsorted = 1;
      }
    }
  }
  //At this point, the leaderboard array should be sorted by point total
  //Prepares to send out the leaderboard data to the clients
  header_t header;
  if(end_game) {
    header.message_type = 'f';
  } else {
    header.message_type = 'l';
  }
  header.num_users = usermap->num_users;
  for(int i = 0; i < usermap->num_users; i++) {
    //Sends out our header
    write(leaderboard[i]->fd, &header, sizeof(header_t));
    //Sends out the answers to the question 
    write(leaderboard[i]->fd, &leaderboard[i]->answers[question_num], sizeof(int));
    //Sends out the correct answer to the user
    write(leaderboard[i]->fd, &questions_log[question_num].correct_answer_index, sizeof(int));
    //Send out leaderboard info to our clients
    for(int j = 0; j < usermap->num_users; j++) {
      //Sends out each individual user information (username and points)
      int username_len = strlen(leaderboard[j]->username);
      write(leaderboard[i]->fd, &username_len, sizeof(int));
      write(leaderboard[i]->fd, leaderboard[j]->username, strlen(leaderboard[j]->username));
      write(leaderboard[i]->fd, &leaderboard[j]->points, sizeof(int));
    }
  }
}
//Calculates our score, we give 500 points for getting the question right, and the remaining points "decay" over time
int score(long answer_time, long total_time) {
  return 1000 - (int)(500L*(total_time-answer_time)/(double)total_time);
}
//Listens for responses to questions from clients and adds in new clients if needed
void* listen_msg(void* p){
  args_t * client_args = (args_t*)p;
  //Makes sure you don't overindex by one spot in the user array 
  int index = client_args->id % users.map_size;
  while(users.users_arr[index]->fd != client_args->id) {
    index++;
  }
  //Run an infinite loop
  while(true){
    response_header_t response_header;
    //If we read in a header file, we actually do things
    if(read(users.users_arr[index]->fd, &response_header, sizeof(response_header_t)) > 0) {
      int username_len = response_header.username_len;
      char response_type = response_header.response_type;

      char username[username_len+1];
      read(users.users_arr[index]->fd, username, username_len);
      username[username_len] = '\0';
      //If we have a '!' flag, that means we have a new user
      if(response_type == '!') {
        users.users_arr[index]->username = strdup(username);
        printf("%s connected!\n", users.users_arr[index]->username);
        //If we read in an answer to a question, we will have the 'a' flag
      } else if(response_type == 'a') {
      	int question_number;
      	read(users.users_arr[index]->fd, &question_number, sizeof(int));
      	char answer;
      	read(users.users_arr[index]->fd, &answer, sizeof(char));
        long time_sec;
        read(users.users_arr[index]->fd, &time_sec, sizeof(long));
        //Checks for answer correctness
        if(answer - 'a' == questions_log[question_number].correct_answer_index || answer - 'A' == questions_log[question_number].correct_answer_index) {
          users.users_arr[index]->answers[question_number] = 1;
          users.users_arr[index]->points = users.users_arr[index]->points + score(time_sec, 20L);

        } else {
          //The person got it wrong
          users.users_arr[index]->answers[question_number] = -1;
        }
        //If we don't get a good flag
      } else {
        printf("Thread %d recieved a garbage response type from %s, it looks like %c\n", users.users_arr[index]->fd, username, response_type);
      }
    }
  }
}


//Creates a new user and puts it into the array
void new_user(int client_socket_fd) {
  int index = client_socket_fd % users.map_size;
  user_t* new_user = (user_t*)malloc(sizeof(user_t));
  new_user->fd = client_socket_fd;
  new_user->answers = malloc(TOTAL_QUESTIONS*sizeof(int));
  memset(new_user->answers, 0 , TOTAL_QUESTIONS);
  new_user->points = 0;
  while(users.users_arr[index] != 0) {
    index++;
  }
  users.users_arr[index] = new_user;
  users.num_users++;
}


void* listen_for_new_connections(void* p){
  args_t * server_args = (args_t*)p;
  
  // Loop that waits and accepts connections by adding them to the global array
  for(int i = 0; i < DEFAULT_MAP_SIZE; i++){
    // Wait for a client to connect
    int client_socket_fd = server_socket_accept(server_args->id);
    if(client_socket_fd == -1) {
      perror("accept failed");
      exit(2);
    }
    // Aquiring the lock, adding the client to the array and releasing the lock
    pthread_mutex_lock(&new_connection_lock);
    new_user(client_socket_fd);
    args_t client_args;
    client_args.id = client_socket_fd;
    pthread_create(&connection_threads[users.num_users], NULL, listen_msg, (void*)&client_args);
    pthread_mutex_unlock(&new_connection_lock);
  }
  return NULL;
}

void play_game(json_t* questions) {
  srand(time(NULL));
  int num_questions = json_array_size(questions);
  for(int i = 0; i < num_questions; i++) {
    json_t *data, *category, *difficulty, *question, *correct_answer, *incorrect_answers;
    char *question_text;
    char *category_text;
    char *difficulty_text;
    char *correct_answer_text;
    char *incorrect_answers_text;
    //Reads in json questions
    data = json_array_get(questions, i);
    if(!json_is_object(data))
      {
        fprintf(stderr, "error: commit data %d is not an object\n", i + 1);
        json_decref(questions);
        return;
      }
    //Reads in json categories
    category = json_object_get(data, "category");
    if(!json_is_string(category))
      {
        fprintf(stderr, "error: commit %d: category is not a string\n", i + 1);
        json_decref(questions);
        return;
      }

    //Reads in difficulty 
    difficulty = json_object_get(data, "difficulty");
    if(!json_is_string(difficulty))
      {
        fprintf(stderr, "error: commit %d: difficulty is not a string\n", i + 1);
        json_decref(questions);
        return;
      }

    //Reads in the question
    question = json_object_get(data, "question");
    if(!json_is_string(question))
      {
        fprintf(stderr, "error: commit %d: question is not a string\n", i + 1);
        json_decref(questions);
        return;
      }
    //Reads in the right answer
    correct_answer = json_object_get(data, "correct_answer");
    if(!json_is_string(correct_answer))
      {
        fprintf(stderr, "error: commit %d: correct_answer is not a string\n", i + 1);
        json_decref(questions);
        return;
      }

    //Reads in the wrong answers
    incorrect_answers = json_object_get(data, "incorrect_answers");
    if(!json_is_array(incorrect_answers))
      {
        fprintf(stderr, "error: commit %d: incorrect_answers is not a string\n", i + 1);
        json_decref(questions);
        return;
      }
    //Just assigns the json text to more concrete variables
    question_t new_question;
    new_question.question_text = strdup(json_string_value(question));
    int question_len = strlen(new_question.question_text);
    new_question.difficulty_text = strdup(json_string_value(difficulty));
    int difficulty_len = strlen(new_question.difficulty_text);
    new_question.category_text = strdup(json_string_value(category));
    int category_len = strlen(new_question.category_text);
    correct_answer_text = strdup(json_string_value(correct_answer));
    int correct_answer_len = strlen(correct_answer_text);
    header_t header;
    header.message_type = 'q';
    cur_question_num = i;
    cur_correct_index = rand() % 4;
    new_question.correct_answer_index = cur_correct_index;
    //Writes to all the users
    for(int j = 0; j < users.map_size; j++) {
      if(users.users_arr[j] != 0) {
        int target_fd = users.users_arr[j]->fd;
        //Send header denoting a question
        write(target_fd, &header, sizeof(header));
        //Send question number
        write(target_fd, &cur_question_num, sizeof(int));
        //Send question metadata
        write(target_fd, &category_len, sizeof(int));
        write(target_fd, new_question.category_text, category_len);
        write(target_fd, &difficulty_len, sizeof(int));
        write(target_fd, new_question.difficulty_text, difficulty_len);
        write(target_fd, &question_len, sizeof(int));
        write(target_fd, new_question.question_text, question_len);
        int incorrect_index = 0;
        //Send all the answers, randomizes them too
        for(int x = 0; x < 4; x++) {
          new_question.answers[x] = strdup(correct_answer_text);
          if(x == cur_correct_index) {
            write(target_fd, &correct_answer_len, sizeof(int));
            write(target_fd, new_question.answers[x], correct_answer_len);
          } else {
            json_t* incorrect = json_array_get(incorrect_answers, incorrect_index);
            if(!json_is_string(incorrect)) {
              fprintf(stderr, "error: commit %d: correct_answer is not a string\n", x + 1);
              json_decref(questions);
            }
            new_question.answers[x] = strdup(json_string_value(incorrect));
            
            int cur_incorrect_answer_len = strlen(new_question.answers[x]);
            write(target_fd, &cur_incorrect_answer_len, sizeof(int));
            write(target_fd, new_question.answers[x], cur_incorrect_answer_len);
            incorrect_index++;
          }
        }
        questions_log[i] = new_question;
      }
    }
    //Question is sent, now wait for question duration before sending next question
    struct timespec question_timer;
    question_timer.tv_sec  = 20L;
    question_timer.tv_nsec = 0;
    nanosleep(&question_timer, NULL);
    report_standing(&users, 0, cur_question_num);
    //Add delay for users to see leaderboard.
    struct timespec leaderboard_timer;
    leaderboard_timer.tv_sec  = 5L;
    leaderboard_timer.tv_nsec = 0;
    nanosleep(&leaderboard_timer, NULL);
  }
}
//Prints off the player names
void print_players() {
  for(int i = 0; i < users.map_size; i++) {
    if(users.users_arr[i] != 0) {
      printf("User %d is named %s\n", i, users.users_arr[i]->username);
    }
  }
}


int main(int argc, char** argv) {
  // Make sure the arguments include a filename
  if(argc != 2 && argc != 4) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    exit(1);
  }
  char* filename = strdup(argv[1]);

  users.num_users = 0;
  users.map_size = DEFAULT_MAP_SIZE;
  users.users_arr = (user_t**)calloc(sizeof(user_t*),DEFAULT_MAP_SIZE);
  
  unsigned short port = 0;
  int server_socket_fd = server_socket_open(&port);
  if(server_socket_fd == -1){
    perror("Server socket was not opened");
    exit(2);
  }

  // Start listening
  if(listen(server_socket_fd, 1)) {
    perror("listen failed");
    exit(2);
  }

  // After listening we have to accept the incoming connections through the use of threads
  pthread_t new_connection_thread;
  args_t server_args;
  server_args.id = server_socket_fd;
  pthread_create(&new_connection_thread, NULL, listen_for_new_connections, (void*)&server_args);

  printf("Running on port %d\n", port);
  char input;
  printf("Enter 's' when you are ready to start the game\n");
  while((input = getchar()) != 's') {
  	
  } 
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  json_t *root;
  json_error_t error;
  root = json_load_file(filename, 0, &error);

  if(!root)
    {
      fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
    }
  json_t *questions;
  questions = json_object_get(root, "results");
  TOTAL_QUESTIONS = json_array_size(questions);
  questions_log = malloc(sizeof(question_t)*TOTAL_QUESTIONS);
  if(!json_is_array(questions))
    {
      fprintf(stderr, "error: questions is not an array\n");
      json_decref(questions);
    }
  // Then wait for user to tell server to begin, or wait a set amount of time before beginning
  play_game(questions);
  clock_gettime(CLOCK_MONOTONIC, &end);
  printf("The amount of time the server took was %ld seconds and %ld milliseconds\n", end.tv_sec-start.tv_sec, end.tv_nsec/1000000L);
  return 0;
}
