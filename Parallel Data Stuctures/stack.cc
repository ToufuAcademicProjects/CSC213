#include "stack.hh"



// Initialize a stack
void stack_init(my_stack_t* stack)
{
  pthread_mutex_init(&stack->lock, NULL);
  stack->root = NULL;
}

// Destroy a stack
void stack_destroy(my_stack_t* stack)
{
  pthread_mutex_lock(&stack->lock);
  while(stack->root != NULL)
    {
      node_t * temp = stack->root;
      stack->root = stack->root->next;
      free(temp);
    }
  pthread_mutex_unlock(&stack->lock);

}

// Push an element onto a stack
void stack_push(my_stack_t* stack, int element)
{
  pthread_mutex_lock(&stack->lock);
  node_t* new_node = (node_t*) malloc(sizeof(node_t));
  new_node->next = stack->root;
  new_node->val = element;
  stack->root = new_node;
  pthread_mutex_unlock(&stack->lock);
}

// Check if a stack is empty
bool stack_empty(my_stack_t* stack)
{
  pthread_mutex_lock(&stack->lock);
  if(stack->root == NULL)
    {
      pthread_mutex_unlock(&stack->lock);
      return true;
    }
  else
    {
      pthread_mutex_unlock(&stack->lock);
      return false;
    }
}

// Pop an element off of a stack
int stack_pop(my_stack_t* stack)
{
  int data = -1;
  pthread_mutex_lock(&stack->lock);

  if(stack->root!=NULL)
    {
      node_t* temp = stack->root;
      //Set our start to the next node
      stack->root = stack->root->next;
      //Store our value and then free it
      data = temp->val;
      free(temp);
      pthread_mutex_unlock(&stack->lock);
      return data;
    }
  else
    {
      pthread_mutex_unlock(&stack->lock);
      return -1;
    }

}
