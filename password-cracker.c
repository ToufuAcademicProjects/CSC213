#define _GNU_SOURCE
#include <openssl/md5.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#define MAX_USERNAME_LENGTH 64
#define PASSWORD_LENGTH 6

#define POW1 26
#define POW2 676
#define POW3 17576
#define POW4 456976
#define POW5 11881376
#define POW6 308915776

#define num_threads 4


/************************* Part A *************************/

/**
 * Find a six character lower-case alphabetic password that hashes
 * to the given hash value. Complete this function for part A of the lab.
 *
 * \param input_hash  An array of MD5_DIGEST_LENGTH bytes that holds the hash of a password
 * \param output      A pointer to memory with space for a six character password + '\0'
 * \returns           0 if the password was cracked. -1 otherwise.
 */
int crack_single_password(uint8_t* input_hash, char* output) {
  long count = 0;
  char result[PASSWORD_LENGTH + 1]; //< This variable holds the password we are trying
  result[6] = '\0';

  while(count<POW6)
    {
      int tempcount = count;
      // Take our candidate password and hash it using MD5
      int a = 5;
      while(a>=0)
        {
          result[a] = 'a' + count%POW1;
          count = count/POW1;
          a--;
        }
      count = tempcount;
      uint8_t candidate_hash[MD5_DIGEST_LENGTH]; //< This will hold the hash of the candidate password
      MD5((unsigned char*)result, PASSWORD_LENGTH, candidate_hash); //< Do the hash
      // Now check if the hash of the candidate password matches the input hash
      if(memcmp(input_hash, candidate_hash, MD5_DIGEST_LENGTH) == 0) {
        // Match! Copy the password to the output and return 0 (success)
        strncpy(output, result, PASSWORD_LENGTH+1);
        return 0;
      } else {
        //Make a new password
        count++;
      }
    }
  return -1;
}
//This converts our count number to a string
char* translate(long count)
{
  char* result = malloc(sizeof(char)*(PASSWORD_LENGTH+1));
  int a = 5;
  while(a>=0)
    {
      result[a] = 'a' + count%POW1;
      count = count/POW1;
      a--;
    }
  return result;
}


//Returns 0 if it didn't work, 1 otherwise, only looks at a single number and converts it into a try
int check_pass(uint8_t* hash, uint8_t* candidate_hash)
{
 
  if(memcmp(hash, candidate_hash, MD5_DIGEST_LENGTH ) == 0)
    {
      // Match, return 1
      return 1;
    }
  else
    {
      //Make a new password, return 0
      return 0;
    }
  
}

/********************* Parts B & C ************************/

/**
 * This struct is the root of the data structure that will hold users and hashed passwords.
 * This could be any type of data structure you choose: list, array, tree, hash table, etc.
 * Implement this data structure for part B of the lab.
 */


typedef struct password_node
{
  char* username;
  uint8_t* hash;
  struct password_node* next;
  int cracked;
  pthread_mutex_t lock;
} password_node_t;

typedef struct password_set {
  password_node_t* start;
  int remainder;
  pthread_mutex_t lock;

} password_set_t;




typedef struct args
{
  password_set_t* passwords;
  long count;
  long max;
} args_t;




/**
 * Initialize a password set.
 * Complete this implementation for part B of the lab.
 *
 * \param passwords  A pointer to allocated memory that will hold a password set
 */
void init_password_set(password_set_t* passwords) {
  // TODO: Initialize any fields you add to your password set structure
  //passwords->hashtable = malloc(sizeof(password_node_t*)*26);

  pthread_mutex_init( &passwords->lock, NULL);

  passwords->start = NULL;
  passwords->remainder = 0;
}

/**
 * Add a password to a password set
 * Complete this implementation for part B of the lab.
 *
 * \param passwords   A pointer to a password set initialized with the function above.
 * \param username    The name of the user being added. The memory that holds this string's
 *                    characters will be reused, so if you keep a copy you must duplicate the
 *                    string. I recommend calling strdup().
 * \param password_hash   An array of MD5_DIGEST_LENGTH bytes that holds the hash of this user's
 *                        password. The memory that holds this array will be reused, so you must
 *                        make a copy of this value if you retain it in your data structure.
 */
void add_password(password_set_t* passwords, char* username, uint8_t* password_hash) {
  // TODO: Add the provided user and password hash to your set of passwords
  uint8_t* pass = malloc(MD5_DIGEST_LENGTH);
  memcpy(pass, password_hash ,MD5_DIGEST_LENGTH );
  password_node_t* temp = passwords->start;
  if(temp == NULL)
    {
      temp = malloc(sizeof(password_node_t));
      temp->username = strdup(username);
      temp->hash = pass;
      //0 means the password hasn't been cracked
      temp->cracked = 0;
      temp->next = NULL;
      pthread_mutex_init( &temp->lock, NULL);
      passwords->start = temp;
    }
  else
    {  
      while(temp->next != NULL)
        {
          temp = temp->next;
        }
      //When this exits, the temp->next node wil be null
      temp->next = malloc(sizeof(password_node_t));
      temp->next->username = strdup(username);
      temp->next->hash = pass;
      pthread_mutex_init( &temp->lock, NULL);
      temp->next->next = NULL;

    }
  passwords->remainder++;
}

/**
 * Crack all of the passwords in a set of passwords. The function should print the username
 * and cracked password for each user listed in passwords, separated by a space character.
 * Complete this implementation for part B of the lab.
 *
 * \returns The number of passwords cracked in the list
 */

//Partitionsa are at, aaaaaa, gnaaaa, tnaaaa, naaaaa
void* crack_password_list_chunk(void* arg)
{
  args_t* stuff = (args_t*)arg;
  password_set_t* passwords = stuff->passwords;
  long count = stuff->count;
  long max = stuff->max;
  int left = 999;
  //printf("Our beginning is %ld\n", count);
  //int cracked = passwords->remainder;
  while(left > 0 && passwords->start != NULL && count < max)
    {
      //char* ss = translate(count);
      //printf("Count is: %s\n", ss );
      
      //pthread_mutex_lock(&passwords->lock);

      /* if(passwords->start == NULL  || passwords->remainder <= 0) */
      /*   { */
      /*     pthread_mutex_unlock(&passwords->lock); */
      /*     return NULL; */
      /*   } */
      //pthread_mutex_unlock(&passwords->lock);

      //Go through all of our passwords and check whether they are the same
      password_node_t* temp = passwords->start;
      password_node_t* prev = temp;

      long counta = count;
      char result[PASSWORD_LENGTH + 1]; //< This variable holds the password we are trying
      result[6] = '\0';
      int a = 5;
      while(a>=0)
        {
          result[a] = 'a' + counta%POW1;
          counta = counta/POW1;
          a--;
        }
      //At this point, result will be a string the corresponds to count
      uint8_t candidate_hash[MD5_DIGEST_LENGTH]; //< This will hold the hash of the candidate password
      MD5((unsigned char*)result, PASSWORD_LENGTH, candidate_hash); //< Do the hash

      
      int l = check_pass(temp->hash, candidate_hash);
      //This is the special case where our linked list start node is our cracked password
      if(l==1)
        {
          
          char* stu = translate(count);
          printf("%s %s\n", temp->username, stu);
          pthread_mutex_lock(&passwords->lock);
          passwords->remainder--;
          pthread_mutex_unlock(&passwords->lock);

          passwords->start->cracked = 1;
          
          //Checks if our node is the last node, we will return null if it is
          if(passwords->start->next == NULL)
            {
              //printf("count is: %ld", count);

              return NULL;
            }
          //pthread_mutex_lock(&passwords->start->lock);
          //pthread_mutex_lock(&passwords->start->next->lock);
          
          //passwords->start = passwords->start->next;
          //pthread_mutex_unlock(&passwords->start->lock);

        }
      
      temp = temp->next;
      int aaa = 0;
      while(temp!=NULL)
        {
          aaa++;
          //Get the locks for prev and temp
          //pthread_mutex_lock(&temp->lock);
          //pthread_mutex_lock(&prev->lock);
          if(temp->cracked == 1)
            {
            }
          else
            {
              l = check_pass(temp->hash,  candidate_hash);
            
              if(l==1)
                {
                  pthread_mutex_lock(&passwords->lock);
                  passwords->remainder--;
                  pthread_mutex_unlock(&passwords->lock);


                  char* stu = translate(count);
                  printf("%s %s\n", temp->username, stu);     // printf("%d\n", aaa);

                  //Now we edit the list to take out the node
                  temp->cracked = 1;
                  //prev->next = temp->next;

                }
            }
          //Unlock the locks for prev and temp
          //pthread_mutex_unlock(&temp->lock);
          //pthread_mutex_unlock(&prev->lock);
          prev = temp;
          temp = temp->next;
        }
                  
      count++;

      pthread_mutex_lock(&passwords->lock);
      left = passwords->remainder;
      pthread_mutex_unlock(&passwords->lock);

    }
  //printf("count is: %ld", count);
  return NULL;
}



int crack_password_list(password_set_t* passwords)
{
  //int a = crack_password_list_chunk(passwords, 0);
  int starting_count = passwords->remainder;
  long partition = POW6/num_threads;
  int b = 0;
  long cur = 0;
  pthread_t threads[num_threads];
  
  while(b<num_threads)
    {
      args_t* arguments = malloc(sizeof(args_t));
      arguments->passwords = passwords;
      arguments->count = cur;
      arguments->max = cur+partition;
      pthread_create(&threads[b], NULL, &crack_password_list_chunk, arguments);
      cur = cur + partition;
      //printf("cur is %ld\n", cur);
      b++;
    }
  int c = 0;
    

  while(c<num_threads)
    {
      pthread_join(threads[c], NULL);
      c++;
    }
  return starting_count - passwords->remainder;
}



/******************** Provided Code ***********************/

/**
 * Convert a string representation of an MD5 hash to a sequence
 * of bytes. The input md5_string must be 32 characters long, and
 * the output buffer bytes must have room for MD5_DIGEST_LENGTH
 * bytes.
 *
 * \param md5_string  The md5 string representation
 * \param bytes       The destination buffer for the converted md5 hash
 * \returns           0 on success, -1 otherwise
 */
int md5_string_to_bytes(const char* md5_string, uint8_t* bytes) {
  // Check for a valid MD5 string
  if(strlen(md5_string) != 2 * MD5_DIGEST_LENGTH) return -1;
  
  // Start our "cursor" at the start of the string
  const char* pos = md5_string;
  
  // Loop until we've read enough bytes
  for(size_t i=0; i<MD5_DIGEST_LENGTH; i++) {
    // Read one byte (two characters)
    int rc = sscanf(pos, "%2hhx", &bytes[i]);
    if(rc != 1) return -1;
    
    // Move the "cursor" to the next hexadecimal byte
    pos += 2;
  }
  
  return 0;
}

void print_usage(const char* exec_name) {
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  %s single <MD5 hash>\n", exec_name);
  fprintf(stderr, "  %s list <password file name>\n", exec_name);
}

int main(int argc, char** argv) {
  if(argc != 3) {
    print_usage(argv[0]);
    exit(1);
  }
  
  if(strcmp(argv[1], "single") == 0) {
    // The input MD5 hash is a string in hexadecimal. Convert it to bytes.
    uint8_t input_hash[MD5_DIGEST_LENGTH];
    if(md5_string_to_bytes(argv[2], input_hash)) {
      fprintf(stderr, "Input has value %s is not a valid MD5 hash.\n", argv[2]);
      exit(1);
    }
    
    // Now call the crack_single_password function
    char result[7];
    if(crack_single_password(input_hash, result)) {
      printf("No matching password found.\n");
    } else {
      printf("%s\n", result);
    }
    
  } else if(strcmp(argv[1], "list") == 0) {
    // Make and initialize a password set
    password_set_t passwords;
    init_password_set(&passwords);
    
    // Open the password file
    FILE* password_file = fopen(argv[2], "r");
    if(password_file == NULL) {
      perror("opening password file");
      exit(2);
    }
  
    int password_count = 0;
  
    // Read until we hit the end of the file
    while(!feof(password_file)) {
      // Make space to hold the username
      char username[MAX_USERNAME_LENGTH];
      
      // Make space to hold the MD5 string
      char md5_string[MD5_DIGEST_LENGTH * 2 + 1];
      
      // Make space to hold the MD5 bytes
      uint8_t password_hash[MD5_DIGEST_LENGTH];

      // Try to read. The space in the format string is required to eat the newline
      if(fscanf(password_file, "%s %s ", username, md5_string) != 2) {
        fprintf(stderr, "Error reading password file: malformed line\n");
        exit(2);
      }

      // Convert the MD5 string to MD5 bytes in our new node
      if(md5_string_to_bytes(md5_string, password_hash) != 0) {
        fprintf(stderr, "Error reading MD5\n");
        exit(2);
      }
      
      // Add the password to the password set
      add_password(&passwords, username, password_hash);
      password_count++;
    }
    
    // Now run the password list cracker
    int cracked = crack_password_list(&passwords);
    
    printf("Cracked %d of %d passwords.\n", cracked, password_count);
    
  } else {
    print_usage(argv[0]);
    exit(1);
  }

  return 0;
}
