/*this program shuffles and prints a deck of cards semi-randomly*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if defined(_WIN32) || defined(__MSDOS__)
   #define SPADE   "\x06"
   #define CLUB    "\x05"
   #define HEART   "\x03"
   #define DIAMOND "\x04"
#else
   #define SPADE   "\xE2\x99\xA0"
   #define CLUB    "\xE2\x99\xA3"
   #define HEART   "\xE2\x99\xA5"
   #define DIAMOND "\xE2\x99\xA6"
#endif

//this helper function takes an integer and assign a suit and value according to the integer
//we assign 0 to be king, 1 to be A, 11 to be J and 12 to be Q, after we do i%13 
void helper (int i) {

  int suit = i % 4;
  int number = i % 13;
  
   if (suit == 0){
      printf(SPADE);
    }
    else if (suit == 1){
      printf(CLUB);
    }
    else if (suit == 2){
      printf(HEART);
    }
    else{
      printf(DIAMOND);
    }

    if (number == 0){
      printf("K\n");
    }
    else if (number == 1){
      printf("A\n");
    }
    else if (number == 11){
      printf("J\n");
    }
    else if (number == 12){
      printf("Q\n");
    }
    else {
      printf("%d\n", number);
    }
}


int main(int argc, char** argv) {

  //creates an array representing the card index
  int boolArr[52];
  for (int l =0; l< 51; l++)
    {
      boolArr[l] = 0;
    }

  //assign the random seed
  srand(time(0));
  int totalPrinted = 0;

  while(totalPrinted<=51)
    {
      //randomly chooses a nnumber from 0 to 51 and pass that number into helper
      int num = rand()%52;
      //checks if we have taken that number, if yes then run rand() again
      if(boolArr[num] == 0)
        {
          helper(num);
          boolArr[num] = 1;
          totalPrinted++;
        }
    }
  
  
  return 0;
}
