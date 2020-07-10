#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_contents(uint8_t* data, size_t size);

                         



int main(int argc, char** argv) {
  // Make sure we have a file input
  if(argc != 2) {
    fprintf(stderr, "Please specify an input filename.\n");
    exit(1);
  }
  
  // Try to open the file
  FILE* input = fopen(argv[1], "r");
  if(input == NULL) {
    perror("Unable to open input file");
    exit(1);
  }
  
  // Seek to the end of the file so we can get its size
  if(fseek(input, 0, SEEK_END) != 0) {
    perror("Unable to seek to end of file");
    exit(2);
  }
  
  // Get the size of the file
  size_t size = ftell(input);
  
  // Seek back to the beginning of the file
  if(fseek(input, 0, SEEK_SET) != 0) {
    perror("Unable to seek to beginning of file");
    exit(2);
  }
  
  // Allocate a buffer to hold the file contents. We know the size in bytes, so
  // there's no need to multiply to get the size we pass to malloc in this case.
  uint8_t* data = malloc(size);
  
  // Read the file contents
  if(fread(data, 1, size, input) != size) {
    fprintf(stderr, "Failed to read entire file\n");
    exit(2);
  }
  
  // Make sure the file starts with the .ar file signature
  if(memcmp(data, "!<arch>\n", 8) != 0) {
    fprintf(stderr, "Input file is not in valid .ar format\n");
    exit(1);
  }
  
  // Call the code to print the archive contents
  print_contents(data, size);
  
  // Clean up
  free(data);
  fclose(input);
  
  return 0;
}

/**
 * This function should print the name of each file in the archive followed by its contents.
 *
 * \param data This is a pointer to the first byte in the file.
 * \param size This is the number of bytes in the file.
 */
void print_contents(uint8_t* data, size_t size)
{
  //We'll use a long here just in case our number is large, this just keeps track of the size easier
  long sizen = (long)size;
  
  //Gets rid of File Signiture
  data = data + 8;
  sizen = sizen - 8;

  //Continue looping until we run out of files 
  while(sizen > 0)
    {
      //Gets the string that encodes the file size of this current file
      char s[10];
      int ne = 48;
      int interator = 0;

      //We get the file size string
      while(interator < 8)
        {
          s[interator] = data[interator+ne];
          interator++;
        }
      s[9] = '\0';
  
      //We extract the file size from the file size string 
      int fileSize = atoi(s) + atoi(s)%2;
  
      //Prints out the file name
      int n = 0;
      while(n<16)
        {
          //Stops printing at the /
          if(data[n] == '/')
            {
              break;
            }
          printf("%c", data[n]);
          n++;
        }
      printf("\n");

      //We skip the header of the file
      data = data+60;
      
      //We find whether our file size is even or odd
      int rem = atoi(s)%2;
      
      //Print out the actual file
      int iter = 0;
      while(iter < fileSize-rem)
        {
          printf("%c", data[iter]);
          iter++;
        }
      
      printf("\n");
      //Update data/sizen values, we move our data pointer 1 farther if we have an odd file size
      data = data + iter + rem;
      sizen = sizen - fileSize - 60;
    }
}
