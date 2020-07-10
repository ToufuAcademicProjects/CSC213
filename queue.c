#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define QUEUE_END -1

// This struct is where you should define your queue datatype. You may need to
// add another struct (e.g. a node struct) depending on how you choose to
// implement your queue.
typedef struct Node
{
  int data;
  struct Node* next;
} node_t;


typedef struct queue
{
  node_t* start;
  node_t* end;
} queue_t;



/**
 * Initialize a queue pointed to by the parameter q.
 * \param q  This points to allocated memory that should be initialized to an
 *           empty queue.
 */
void queue_init(queue_t* q)
{
  //We just set start and end to a null node, our put function does most of our work for us
  node_t* starter = NULL;
  q->start = starter;
  q->end = starter;
}

/**
 * Add a value to a queue.
 * \param q       Points to a queue that has been initialized by queue_init.
 * \param value   The integer value to add to the queue
 */
void queue_put(queue_t* q, int value)
{
  node_t * temp = q->start;
  //This replaces the NULL value node if our list node is NULL
  if(temp == NULL)
    {
      node_t* starter = malloc(sizeof(node_t));
      q->start = starter;
      q->end = starter;
      starter->data = value;
    }
  //This adds on a new node onto an already existing list
  else
    {
      node_t *nod = malloc(sizeof(node_t));
      nod->data = value;
      q->end->next = nod;
      q->end = q->end->next;
    }
}

/**
 * Check if a queue is empty.
 * \param q   Points to a queue initialized by queue_init.
 * \returns   True if the queue is empty, otherwise false.
 */
bool queue_empty(queue_t* q)
{
  if(q->start == NULL)   return true;
  return false;
}

/**
 * Take a value from a queue.
 * \param q   Points to a queue initialized by queue_init.
 * \returns   The value that has been in the queue the longest time. If the
 *            queue is empty, return QUEUE_END.
 */
int queue_take(queue_t* q)
{
  if(q->start != NULL)
    {
      //Assign temporary variables since we will be getting rid of our starting node
      node_t * temp = q->start;
      int n = temp->data;
      
      //We set the next node as the new starter
      q->start = q->start->next;
      
      // If we are taking in the last value, we'll also need to update end 
      if(q->end == temp)
        {
          q->end = NULL;
        }
      //Free the node memory
      temp->next = NULL;
      free(temp);
      return n;
    }
  return QUEUE_END;
}





/**
 * Free any memory allocated inside the queue data structure.
 * \param q   Points to a queue initialized by queue_init. The memory referenced
 *            by q should *NOT* be freed.
 */
void queue_destroy(queue_t* q)
{
  node_t* temp = q->start;
  //While we still have nodes that aren't destroyed, we free them from their earthly binds
  while(temp != NULL)
    {
      node_t* rid = temp;
      temp = temp->next;
      free(rid);
    }
  //We need to update our end value since everything is freed now
  q->end = NULL;
}

int main(int argc, char** argv)
{
  // Set up and initialize a queue
  queue_t q;
  queue_init(&q);
  
  // Read lines until the end of stdin
  char* line = NULL;
  size_t line_size = 0;
  while(getline(&line, &line_size, stdin) != EOF) {
    int num;
    
    // If the line has a take command, take a value from the queue
    if(strcmp(line, "take\n") == 0) {
      if(queue_empty(&q)) {
        printf("The queue is empty.\n");
      } else {
        printf("%d\n", queue_take(&q));
      }
    } else if(sscanf(line, "put %d\n", &num) == 1) {
      queue_put(&q, num);
    } else {
      printf("unrecognized command.\n");
    }
  }
  
  // Free the space allocated by getline
  free(line);
  
  // Clean up the queue
  queue_destroy(&q);
  
  return 0;
}
