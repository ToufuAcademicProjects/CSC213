#include "uniquelist.h"

#include <stdio.h>
#include <stdlib.h>

/// Initializes a new uniquelist with a NULL starting node
void uniquelist_init(uniquelist_t* s)
{
  node_t * starter = NULL;
  starter = malloc(sizeof(node_t));
  s->start = starter;
}

/// Destroy a uniquelist
void uniquelist_destroy(uniquelist_t* s)
{
  node_t *temp = s->start;
  //free(s->start);
  //free(temp);
  node_t *previous = temp;

  while(temp!=NULL)
    {
      previous = temp;
      temp = temp->next;
      //printf("\nhi\n");

      free(previous);
    }
}

/// Add an element to a uniquelist, unless it's already in the uniquelist
void uniquelist_insert(uniquelist_t* s, int n)
{
  node_t *temp = s->start;
  //Initializes the next node of the uniquelist if it is NULL
  while(0==0)
    {
      //Case when we already have our number in our list
      if(temp->next != NULL && temp->next->data == n)
        {
          return;
        }
      //Case where we have a new number and we reach the end
      else if(temp->next == NULL)
        {
          //Gettinig our node malloc'd, setting the data in it to data
          temp->next = malloc(sizeof(node_t));
          temp->next->data = n;
          temp->next->next = NULL;

          return;
        }
      else
        {
          temp = temp->next;
        }
    }
}

/// Print all the numbers in a uniquelist
void uniquelist_print_all(uniquelist_t* s)
{
  node_t *temp = s->start;
  temp = temp->next;
  while(temp != NULL)
    {
      printf("%d ", temp->data);
      temp = temp->next;
    }
  printf("\n");
}
