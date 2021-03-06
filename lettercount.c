#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#define PAGE_SIZE 0x1000
#define ROUND_UP(x,y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

//Struct for a single character and the count for it, includes a lock for it
typedef struct kar
{
  char car;
  long count;
  pthread_mutex_t lock;
} kar_t;

//Struct for passing our file_data and file_size into our count_chunk method
typedef struct args
{
  char* file_data;
  off_t file_size;
} args_t;


/// The number of times we've seen each letter in the input, initially zero
kar_t letter_counts[26];

//Counts the occurences of letters in a chunk
void* count_chunk(void* input)
{
  args_t* stuff = (args_t*)input;
  char* file_data = stuff->file_data;
  off_t file_size = stuff->file_size;

  for(size_t i = 0; i< file_size; i++) {
    char c = file_data[i];
    
    if(c >= 'a' && c <= 'z')
      {
        pthread_mutex_lock(&letter_counts[c-'a'].lock);
        letter_counts[c - 'a'].count++;
        pthread_mutex_unlock(&letter_counts[c-'a'].lock);

      }
    else if(c >= 'A' && c <= 'Z')
      {
        pthread_mutex_lock(&letter_counts[c-'A'].lock);
        letter_counts[c - 'A'].count++;
        pthread_mutex_unlock(&letter_counts[c-'A'].lock);

      }

  }
  return NULL;
}

//Initializes our lock/count/letter (kar) datastructure
void init()
{
  int a = 0;

  while(a<26)
    {
      letter_counts[a].count = 0;
      letter_counts[a].car = a + 'a';
      pthread_mutex_init( &letter_counts[a].lock, NULL);
      a++;
    }
}


/**
 * This function should divide up the file_data between the specified number of
 * threads, then have each thread count the number of occurrences of each letter
 * in the input data. Counts should be written to the letter_counts array. Make
 * sure you count only the 26 different letters, treating upper- and lower-case
 * letters as the same. Skip all other characters.
 *
 * \param num_threads   The number of threads your program must use to count
 *                      letters. Divide work evenly between threads
 * \param file_data     A pointer to the beginning of the file data array
 * \param file_size     The number of bytes in the file_data array
 */
void count_letters(int num_threads, char* file_data, off_t file_size)
{
  //We initilize our data structure to add in locks and counters
  init();

  //We divide out our file into a roughly even amount of threads
  long partition_size = file_size/num_threads;
  long partitions[num_threads];
  int n = 0;
  while(n < num_threads)
    {
      partitions[n] = partition_size;
      n++;
    }
  //Set the last partition to be (maybe) larger than the rest to account for rounding
  partitions[n-1] = file_size - partition_size*num_threads + partition_size;

  
  pthread_t threads[num_threads];
  int b = 0;
  //We assign our partitions to our threads
  while(b<num_threads)
    {
      args_t* arguments = malloc(sizeof(args_t));
      arguments->file_data = file_data;
      arguments->file_size = partitions[b];
      pthread_create(&threads[b], NULL, &count_chunk, arguments);
      file_data = file_data + partitions[b];
      b++;
    }
  int c = 0;

  //We join together our threads once they're done
  while(c<num_threads)
    {
      pthread_join(threads[c], NULL);
      c++;
    }
  
}

/**
 * Show instructions on how to run the program.
 * \param program_name  The name of the command to print in the usage info
 */
void show_usage(char* program_name) {
  fprintf(stderr, "Usage: %s <N> <input file>\n", program_name);
  fprintf(stderr, "    where <N> is the number of threads (1, 2, 4, or 8)\n");
  fprintf(stderr, "    and <input file> is a path to an input text file.\n");
}

int main(int argc, char** argv) {
  // Check parameter count
  if(argc != 3) {
    show_usage(argv[0]);
    exit(1);
  }
  
  // Read thread count
  int num_threads = atoi(argv[1]);
  if(num_threads != 1 && num_threads != 2 && num_threads != 4 && num_threads != 8) {
    fprintf(stderr, "Invalid number of threads: %s\n", argv[1]);
    show_usage(argv[0]);
    exit(1);
  }
  
  // Open the input file
  int fd = open(argv[2], O_RDONLY);
  if(fd == -1) {
    fprintf(stderr, "Unable to open input file: %s\n", argv[2]);
    show_usage(argv[0]);
    exit(1);
  }
  
  // Get the file size
  off_t file_size = lseek(fd, 0, SEEK_END);
  if(file_size == -1) {
    fprintf(stderr, "Unable to seek to end of file\n");
    exit(2);
  }

  // Seek back to the start of the file
  if(lseek(fd, 0, SEEK_SET)) {
    fprintf(stderr, "Unable to seek to the beginning of the file\n");
    exit(2);
  }
  
  // Load the file with mmap
  char* file_data = mmap(NULL, ROUND_UP(file_size, PAGE_SIZE), PROT_READ, MAP_PRIVATE, fd, 0);
  if(file_data == MAP_FAILED) {
    fprintf(stderr, "Failed to map file\n");
    exit(2);
  }
  
  // Call the function to count letter frequencies
  count_letters(num_threads, file_data, file_size);
  
  // Print the letter counts
  for(int i=0; i<26; i++) {
    printf("%c: %lu\n", 'a' + i, letter_counts[i].count);
  }
  
  return 0;
}
