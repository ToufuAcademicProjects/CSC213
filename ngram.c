#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#define MAX 99999
 

// str = the string we're looking at
// start = the index we start printing from
// chars = the number of chars we're printing
// Return value = not useful
int print(char *str, int start, int chars)
{
  char ret[chars+1];
  int count = 0;
  while(count < chars)
    {
      ret[count] = str[start+count];
      count++;
    }
  ret[chars] = '\0';
  if(ret[chars+1] != EOF)
    {
      printf("%s\n", ret);
    }
  return start;
}
//Same thing but doesn't put a new line at the end. currently not used
//returning -1 when an eof is found
int print_over_max(char *str, int start, int chars)
{
  char ret[chars+1];
  int count = 0;
  while(count < chars)
    {
      ret[count] = str[start+count];
      count++;
    }
  if(ret[chars+1] != EOF)
    {
      printf("%s", ret);
      return start;
    }
  return -1;
}

int main(int argc, char** argv)
{


  
  // Make sure the program is run with an N parameter
  if(argc != 2) {
    fprintf(stderr, "Usage: %s N (N must be >= 1)\n", argv[0]);
    exit(1);
  }
  
  // Convert the N parameter to an integer
  int N = atoi(argv[1]);
  
  // Make sure N is >= 1
  if(N < 1) {
    fprintf(stderr, "Invalid N value %d\n", N);
    exit(1);
  }
  
  char test[MAX];
  fgets(test, MAX, stdin);
  


  //Case where N is larger than our MAX, this shouldn't really happen...
  if(N > MAX)
    {
      //int times = N/MAX;
      //int reps = 0;
      //Work on this ----------------------------------------------------
      return 0;
    }
  else
    {
      //Put in first case code in here
      int gram = N;
      int start = 0;
      int end = strlen(test)-(gram-1);
      while(start<end)
        {
          print(test, start, gram);
          start++;
        }

      //If we don't finish our input, we continue here with the next buffer
      int contin = 0;
      char test2[MAX];
      char* a  = fgets(test2, MAX, stdin);
      if(a==NULL)
        {
          contin = 1;
        }

      //Takes care of the next buffers
      while(contin == 0)
        {
          //printf("---------------------------");
          //Takes care of the bridge between buffers, adds part of the previous buffer to the current chunk
          char *endChunk;
          endChunk = &test[MAX-gram];
          char test3[MAX+gram]; 
          strcpy(test3, endChunk);
          strcat(test3, test2);
          //printf("\n%s\n", test3);
          //Prints the bulk of the buffer (except for special cases)
          int gram = N;
          int start = 0;
          int end = strlen(test3)-(gram-1);
          while(start<end)
            {
              print(test3, start, gram);
              start++;
            }

          //Helps take care of the bridge, test store the previous bufer, we will replace test2 with the new info.
          strcpy(test, test2);
      
          //Gets the next buffer
          a = fgets(test2, MAX, stdin);
          if(a==NULL)
            {
              contin = 1;
            }
        }
    }
  return 0;
}

