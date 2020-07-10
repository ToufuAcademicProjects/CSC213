#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>

void alarm_handler(int signal, siginfo_t* info, void* ctx)
{
  printf("You have a seg fault! ");

  int messNum = 8; // number of messages to print

  char * mess0 = "You thought your code would run, but it was me, Seg Fault!";
  char * mess1 = "Oh look at that, your code is a failure";
  char * mess2 = "Did you think you could escape me this time?";
  char * mess3 = "I am the thing that keeps you programmers up at night";
  char * mess4 = "C'mon ol' pal, get it together";
  char * mess5 = "Might wanna take a look at that code again, dude";
  char * mess6 = "John Green's new novel: The Fault in Our Segmentation";
  char * mess7 = "99 bugs in the code, 99 bugs in the code, take one down, patch it up, 117 bugs in the code!";

  char * messArr[8] = {mess0, mess1, mess2, mess3, mess4, mess5, mess6, mess7};
  srand(time(NULL));
  int n = rand() % messNum;

  printf("%s\n", messArr[n]);  
  
  exit(2);
}

// Signal handling code here!
__attribute__((constructor)) void init() {
  //printf("This code runs at program startup\n");
  
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_sigaction = alarm_handler;
  sa.sa_flags = SA_SIGINFO;

  sigaction(SIGSEGV, &sa, NULL);
     
  
}
