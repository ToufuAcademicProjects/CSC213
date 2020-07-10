#define DEFAULT_MAP_SIZE 128
#define LIFELINES 1

pthread_t connection_threads[128];

int TOTAL_QUESTIONS; 
typedef struct response_header{
  char response_type;
  int username_len;
}response_header_t;

typedef struct user {
  char* username;
  int points;
  int lifelines;
  int fd;
  int* answers;
} user_t;

typedef struct users_map {
  user_t ** users_arr;
  int num_users;
  int map_size;
} users_map_t;

users_map_t users;

typedef struct question{
  int question_num;
  char* question_text;
  int correct_answer_index;
  char* answers[4];
  char* category_text;
  char* difficulty_text;
} question_t;


typedef struct args {
  int id;
  char* message;
} args_t;

typedef struct header{
  char message_type;
  int num_users;
  int message_len;
  int username_len;
  int ans_lens[4];
} header_t;


int cur_question_num;
int cur_correct_index;
question_t* questions_log;


void report_standing(users_map_t* usermap, int end_game, int question_num);

int score(long answer_time, long total_time);

void* listen_msg(void* p);

void new_user(int client_socket_fd);

void* listen_for_new_connections(void* p);

void play_game(json_t* questions);

void print_playerss();
