#define _GNU_SOURCE  

#include "lazycopy.h"
#include <signal.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   
#include <sys/mman.h>


typedef struct Node
{
  void* org;
  struct Node * next;
} node_t;

typedef struct list
{
  node_t* start;
  int init;
} list_t;

struct list l;
// Later, if either copy is written to you will need to:
// 1. Save the contents of the chunk elsewhere (a local array works well)
// 2. Use mmap to make a writable mapping at the location of the chunk that was written
// 3. Restore the contents of the chunk to the new writable mapping
void alarm_handler(int signal, siginfo_t* info, void* ctx)
{
  //We find the source of the error
  long pointer = (intptr_t)info->si_addr; 
  
  //We find which copy the source is from
  struct Node* temp = l.start;
  long address = (intptr_t)temp->org;
  //While our pointer is not within temp's address bounds, we will look at the next one
  while( !(address+CHUNKSIZE>pointer && address <= pointer) )
    {
      temp = temp->next;
      address = (intptr_t)temp->org;
    }
  //Copy the data over to a temp chunk
  void * temp_mem = malloc(CHUNKSIZE);
  memcpy(temp_mem, temp->org, CHUNKSIZE);


  //Create our new chunk of memory
  void* errortest = mmap(temp->org, CHUNKSIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED | MAP_FIXED, -1, 0);
  //Error check
  if(errortest == (void*)-1)
    {
      printf("mmap failed us \n");
    }
  //Move our memory over to our new physical memory
  memcpy(temp->org, temp_mem, CHUNKSIZE);
}


/**
 * This function will be called at startup so you can set up a signal handler.
 */
void chunk_startup()
{
  // TODO: Implement this function
    
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_sigaction = alarm_handler;
  sa.sa_flags = SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
}

/**
 * This function should return a new chunk of memory for use.
 *
 * \returns a pointer to the beginning of a 64KB chunk of memory that can be read, written, and copied
 */
void* chunk_alloc() {
  // Call mmap to request a new chunk of memory. See comments below for description of arguments.
  void* result = mmap(NULL, CHUNKSIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
  // Arguments:
  //   NULL: this is the address we'd like to map at. By passing null, we're asking the OS to decide.
  //   CHUNKSIZE: This is the size of the new mapping in bytes.
  //   PROT_READ | PROT_WRITE: This makes the new reading readable and writable
  //   MAP_ANONYMOUS | MAP_SHARED: This mapes a new mapping to cleared memory instead of a file,
  //                               which is another use for mmap. MAP_SHARED makes it possible for us
  //                               to create shared mappings to the same memory.
  //   -1: We're not connecting this memory to a file, so we pass -1 here.
  //   0: This doesn't matter. It would be the offset into a file, but we aren't using one.
  
  // Check for an error
  if(result == MAP_FAILED) {
    perror("mmap failed in chunk_alloc");
    exit(2);
  }
  
  // Everything is okay. Return the pointer.
  return result;
}

/**
 * Create a copy of a chunk by copying values eagerly.
 *
 * \param chunk This parameter points to the beginning of a chunk returned from chunk_alloc()
 * \returns a pointer to the beginning of a new chunk that holds a copy of the values from
 *   the original chunk.
 */
void* chunk_copy_eager(void* chunk) {
  // First, we'll allocate a new chunk to copy to
  void* new_chunk = chunk_alloc();
  
  // Now copy the data
  memcpy(new_chunk, chunk, CHUNKSIZE);
  
  // Return the new chunk
  return new_chunk;
}

/**
 * Create a copy of a chunk by copying values lazily.
 *
 * \param chunk This parameter points to the beginning of a chunk returned from chunk_alloc()
 * \returns a pointer to the beginning of a new chunk that holds a copy of the values from
 *   the original chunk.
 */

void* chunk_copy_lazy(void* chunk)
{
  //If our list is null
  if(l.init == 0)
    {
      l.init = 1;
      node_t * starter = NULL;
      starter = malloc(sizeof(node_t));
      l.start = starter;
    }

  struct Node* temp = l.start;
  void* new_chunk;
  new_chunk = mremap(chunk, 0, CHUNKSIZE, MREMAP_MAYMOVE);
  while( temp->next != NULL )
    {
      temp = temp -> next;
    }
  //Our next node will be empty so we make it and add in our original chunk
  temp->next = malloc(sizeof(node_t));
  temp->next->org = chunk;

  temp = temp->next;
  
  //Our next node after that will be empty we add in the copy as a node too
  temp->next = malloc(sizeof(node_t));
  temp->next->org = new_chunk;
          
  //Makes the chunks read-only
  mprotect(chunk, CHUNKSIZE,  PROT_READ);
  mprotect(new_chunk, CHUNKSIZE,  PROT_READ);
  
  return new_chunk;

      
}
  
// Your implementation should do the following:
// -1. Use mremap to create a duplicate mapping of the chunk passed in
  // -2. Mark both mappings as read-only
  // 3. Keep some record of both lazy copies so you can make them writable later.
  //    At a minimum, you'll need to know where the chunk begins and ends.
  
  // Later, if either copy is written to you will need to:
  // 1. Save the contents of the chunk elsewhere (a local array works well)
  // 2. Use mmap to make a writable mapping at the location of the chunk that was written
  // 3. Restore the contents of the chunk to the new writable mapping

