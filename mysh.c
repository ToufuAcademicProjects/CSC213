#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
//A source: https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm

// This is the maximum number of arguments your shell should handle for one command
#define MAX_ARGS 128


void parseCommand(char* line, int blocker);
void split_command(char* str)
{
  char* current_position = str;
  while(true)
    {
      // Call strpbrk to find the next occurrence of a delimeter
      char* delim_position = strpbrk(current_position, "&;");
    
      if(delim_position == NULL)
        {
          // There were no more delimeters.
          parseCommand(str, 1);
          return;
      
        }
      else
        {
          int blocker = 1;
          // There was a delimeter. First, save it.
          if(*delim_position =='&')
            {
              blocker = 0;
            }
          // Overwrite the delimeter with a null terminator so we can print just this fragment
          *delim_position = '\0';
          parseCommand(str, blocker);
          str = delim_position+1;
        }
    
      // Move our current position in the string to one character past the delimeter
      current_position = delim_position + 1;
    }
}



void parseCommand(char* line, int blocker)
{
  const char s[] = " $\n";

  char *token;
  char *args [MAX_ARGS+1];

  // get the first token 
  token = strtok(line, s );  

  char *cd = "cd";
  char *exitCommand = "exit";
  if(token == NULL) {}
  else if(strcmp(token, exitCommand) == 0 ) {}
  else if(strcmp(token, cd) == 0 )
    {
      chdir(strtok(NULL,s));
    }
  else
    {
      int argCount = 0;
      // walk through other tokens 
      while( token != NULL && argCount < MAX_ARGS)
        {
          args[argCount] = token;
          token = strtok(NULL, s);
          argCount++;
        }
      args[argCount] = NULL;
      //We run execvp
      int status = 0;
      pid_t waitres = 0;
      pid_t i = fork();
      if(i == 0)
        {
          execvp(args[0] ,args);
          
        }
      else
        {
          //The case where we have &
          if(blocker == 0)   {}
          else
            {
              waitres = wait(&status);
            }
        }
      
      printf("Child process %d", i);
      printf(" exited with status %d\n", WEXITSTATUS(status)); 
    }
}

int main(int argc, char** argv)
{
  // If there was a command line option passed in, use that file instead of stdin
  if(argc == 2)
    {
      // Try to open the file
      int new_input = open(argv[1], O_RDONLY);
      if(new_input == -1)
        {
          fprintf(stderr, "Failed to open input file %s\n", argv[1]);
          exit(1);
        }
    
      // Now swap this file in and use it as stdin
      if(dup2(new_input, STDIN_FILENO) == -1)
        {
          fprintf(stderr, "Failed to set new file as input\n");
          exit(2);
        }
    }
  int status = 0;
  char* line = NULL;    // Pointer that will hold the line we read in
  size_t line_size = 0; // The number of bytes available in line
  
  // Loop forever
  while(true)
    {
      while(waitpid(-1, &status, WNOHANG) == 0)
        {}
      
      // Print the shell prompt
      printf("$ ");
    
      // Get a line of stdin, storing the string pointer in line
      if(getline(&line, &line_size, stdin) == -1)
        {
          if(errno == EINVAL)
            {
              perror("Unable to read command line");
              exit(2);
            }
          else
            {
              // Must have been end of file (ctrl+D)
              printf("\nShutting down...\n");
        
              // Exit the infinite loop
              break;
            }
        }

      printf("Received command: %s\n", line);
    
      split_command(line);
    }
  if(line != NULL)
    {
      free(line);
    }
  return 0;
}

