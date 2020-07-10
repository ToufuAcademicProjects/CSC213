#include "queue.hh"

#include <stdlib.h>

// Initialize a new queue
void queue_init(my_queue_t* queue) {
  pthread_mutex_init(&queue->lock, NULL);
  queue->start = NULL;
}

// Destroy a queue
void queue_destroy(my_queue_t* queue) {
    pthread_mutex_lock(&queue->lock);

  node_t* temp = queue->start;
  while(temp!=NULL)
    {
      node_t* l = temp;
      temp = temp->next;
      free(l);
    }
    pthread_mutex_unlock(&queue->lock);

}

// Put an element at the end of a queue
void queue_put(my_queue_t* queue, int element) {
  pthread_mutex_lock(&queue->lock);
  
  if(queue->start == NULL)
    {
      queue->start = (node_t*)malloc(sizeof(node_t));
      queue->start->val = element;
      queue->start->next = NULL;
    }
  else
    {
  
      node_t* temp = queue->start;
      while(temp->next!=NULL)
        {
          temp = temp->next;
        }
      temp->next = (node_t*)malloc(sizeof(node_t));
      temp->next->val = element;
      temp->next->next = NULL;

    }
  
  pthread_mutex_unlock(&queue->lock);
}

// Check if a queue is empty
bool queue_empty(my_queue_t* queue) {
  pthread_mutex_lock(&queue->lock);

  if(queue->start == NULL)
    {
      pthread_mutex_unlock(&queue->lock);
      return true;
    } 
  pthread_mutex_unlock(&queue->lock);
  return false;
}

// Take an element off the front of a queue
int queue_take(my_queue_t* queue) {
  pthread_mutex_lock(&queue->lock);
  int element = -1;
  if(queue->start != NULL) {
    node_t* taken_node = queue->start;
    element = taken_node->val;
    queue->start = queue->start->next;
    free(taken_node);
  }
  pthread_mutex_unlock(&queue->lock);
  return element;
}
