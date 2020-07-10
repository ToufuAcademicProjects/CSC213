#define _GNU_SOURCE

#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <math.h>
#include <unistd.h> 
// The minimum size returned by malloc
#define MIN_MALLOC_SIZE 16

// Round a value x up to the next multiple of y
#define ROUND_UP(x,y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

// The size of a single page of memory, in bytes
#define PAGE_SIZE 0x1000
#define MAGIC_NUMBER 5

//For the header of a page
typedef struct Header
{
  int size;
  int magic_num;
} header_t;

typedef struct Node
{
  void* next; 
} node_t;

//Global Variable for our pointers to our start nodes
node_t* lists[8]; 

//A function to return a to the power of b (a^b)
int power(int a, int b)
{
  int s = a;
  
  if(b==0)
    {
      return 1;
    }
      
  
  while(b>1)
    {
      s = s * a;
      b--;
    }
  return s;
}

//We get the size class our input belongs in
//0 = <16 bytes
//1 = <32 bytes
//etc.
//-1 = >4096
int get_size_class_i(int input)
{
  int i = 0;
  int low = 0;
  while(low < input)
    {
      low = 16 * power(2, i);
      i++;
    }
  if(i<=8)
    {
      return i-1;
    }
  else
    {
      return -1;
    }
}


/**
 * Allocate space on the heap.
 * \param size  The minimium number of bytes that must be allocated
 * \returns     A pointer to the beginning of the allocated space.
 *              This function may return NULL when an error occurs.
 */
void* xxmalloc(size_t size)
{
  // Round the size up to the next multiple of the page size
  int l = get_size_class_i(size);
  if(l == -1)
    {
      //We let mmap take care of it
      void* p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

      // Check for errors
      if(p == MAP_FAILED)
        {
          fputs("mmap failed! Giving up.\n", stderr);
          exit(2);
        }
      return p;
    }
  //We will now take care of the allocation with our malloc
  else
    {
      //Gets the case where our list holds no free space
      if(lists[l] == NULL)
        {
          //Reset the size to the size of the partition
          size = 16 * power(2,l);
          //Get the actualy memory
          void* pp = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

          //Create our header
          header_t* head = (header_t*)pp;
          head->size = size;
          head->magic_num = MAGIC_NUMBER;

          //Move our pointer over to the next partition
          pp = pp + size;

          //Cast the new partition to the start node
          node_t* start = (node_t*)pp;
          start->next = pp+size;
          //Assign this into our list
          lists[l] = start;

          //We do the actual partitioning
          int sz = 0;
          node_t* temp = (node_t*)pp;

          //We partition out the rest of the chunk of memory
          while(sz < ((4096-2*size)/size)) 
            {
              pp = pp + size;
              temp = (node_t*)pp;
              temp->next = pp+size;
              sz++;

            }

          temp->next = NULL;
        }
      //We are giving away our first node and setting list to point to the next
      node_t * chunk_start = lists[l];
      node_t * temp = chunk_start->next;
      lists[l] = temp;
      return chunk_start;
    }
}






/**
 * Get the available size of an allocated object
 * \param ptr   A pointer somewhere inside the allocated object
 * \returns     The number of bytes available for use in this object
 */
size_t xxmalloc_usable_size(void* ptr)
{
  intptr_t s = (intptr_t)ptr - (intptr_t)ptr%4096;
 
  char buffer[300]; 
  sprintf(buffer, "i = %ld\n", s);
  
  ptr = (void*)s;
  header_t* head = (header_t*)ptr;

  if(MAGIC_NUMBER != head->magic_num)
    {
      return 0;
    }
  
  return head->size;
}





/**
 * Free space occupied by a heap object.
 * \param ptr   A pointer somewhere inside the object that is being freed
 */
void xxfree(void* ptr)
{
  // Don't free NULL!
  if(ptr == NULL) return;

  //We find our partition size
  int size = xxmalloc_usable_size(ptr);

  //If we have a chunk without a magic number we do nothing
  if(size == 0)
    {
      return;
    }
  //We get the start of our partition that we want to free
  long start = (intptr_t)ptr - (intptr_t)ptr%size;
  ptr = (void*)start;

  //We cast our memory into a node
  node_t* new_start = (node_t*)ptr;

  //We find the list the node belongs in
  int i = get_size_class_i(size);
  new_start->next = lists[i];

  //Adds the node into the proper list
  lists[i] = new_start;
}


