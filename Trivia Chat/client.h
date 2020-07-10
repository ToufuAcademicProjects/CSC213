#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#include "socket.h"
#include "ui.h"

#define TOTAL_QUESTIONS 20

char* username;

pthread_t connection;
int connection_fd;
int username_len;
int cur_time;
int end_time;


typedef struct args{
  char* question;
  char* ans1;
  char* ans2;
  char* ans3;
  char* ans4;
}args_t;

typedef struct header{
	char message_type;
  int num_users;
  int message_len;
  int username_len;
  int ans_lens[4];
}header_t;

typedef struct question{
	int question_num;
	char* question_text;
	char* answers[4];
	char* category_text;
	char* difficulty_text;
} question_t;

typedef struct response_header{
	char response_type;
  int username_len;
}response_header_t;



void send_message(char message, long time);

void* func(void* p);
void wait_for_input(int duration);