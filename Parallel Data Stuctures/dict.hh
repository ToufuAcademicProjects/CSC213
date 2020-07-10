#ifndef DICT_H
#define DICT_H
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct node {
  int val;
  const char* key;
  struct node* next;

} node_t;

typedef struct list {
  node_t* root;
  char starting_char;
  pthread_mutex_t lock;
} list_t;

typedef struct my_dict {
  list_t* list[52];
  pthread_mutex_t lock;

} my_dict_t;

// Initialize a dictionary
void dict_init(my_dict_t* dict);

// Destroy a dictionary
void dict_destroy(my_dict_t* dict);

// Set a value in a dictionary
void dict_set(my_dict_t* dict, const char* key, int value);

// Check if a dictionary contains a key
bool dict_contains(my_dict_t* dict, const char* key);

// Get a value in a dictionary
int dict_get(my_dict_t* dict, const char* key);

// Remove a value from a dictionary
void dict_remove(my_dict_t* dict, const char* key);

#endif
