#ifndef STACK_H
#define STACK_H
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct node {
  int val;
  struct node* next;
} node_t;

typedef struct my_stack {
  node_t* root;
  pthread_mutex_t lock;
} my_stack_t;


// Initialize a stack
void stack_init(my_stack_t* stack);

// Destroy a stack
void stack_destroy(my_stack_t* stack);

// Push an element onto a stack
void stack_push(my_stack_t* stack, int element);

// Check if a stack is empty
bool stack_empty(my_stack_t* stack);

// Pop an element off of a stack
int stack_pop(my_stack_t* stack);

#endif
