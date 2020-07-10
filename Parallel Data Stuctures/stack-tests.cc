#include <gtest/gtest.h>

#include "stack.hh"

/****** Stack Invariants ******/

// Invariant 1
// For every value V that has been pushed onto the stack p times and returned by pop q times, there must be p-q copies of this value on the stack. This only holds if p >= q.

// Invariant 2
// No value should ever be returned by pop if it was not first passed to push by some thread.

// Invariant 3
// If a thread pushes value A and then pushes value B, and no other thread pushes these specific values, A must not be popped from the stack before popping B.

/****** Begin Tests ******/

// A simple test of basic stack functionality
TEST(StackTest, BasicStackOps) {
  // Create a stack
  my_stack_t s;
  stack_init(&s);
  
  // Push some values onto the stack
  stack_push(&s, 1);
  stack_push(&s, 2);
  stack_push(&s, 3);
  
  // Make sure the elements come off the stack in the right order
  ASSERT_EQ(3, stack_pop(&s));
  ASSERT_EQ(2, stack_pop(&s));
  ASSERT_EQ(1, stack_pop(&s));
  
  // Clean up
  stack_destroy(&s);
}

// Another test case
TEST(StackTest, EmptyStack) {
  // Create a stack
  my_stack_t s;
  stack_init(&s);
  
  // The stack should be empty
  ASSERT_TRUE(stack_empty(&s));
  
  // Popping an empty stack should return -1
  ASSERT_EQ(-1, stack_pop(&s));
  
  // Put something on the stack
  stack_push(&s, 0);
  
  // The stack should not be empty
  ASSERT_FALSE(stack_empty(&s));
  
  // Pop the element off the stack.
  // We're just testing empty stack behavior, so there's no need to check the resulting value
  stack_pop(&s);
  
  // The stack should be empty now
  ASSERT_TRUE(stack_empty(&s));
  
  // Clean up
  stack_destroy(&s);
}

// Invariant 1 Test 1
TEST(StackTest, invar1) {
  // Create a stack
  my_stack_t s;
  stack_init(&s);
  
  // The stack should be empty
  ASSERT_TRUE(stack_empty(&s));
  
  // Popping an empty stack should return -1
  ASSERT_EQ(-1, stack_pop(&s));

  int a = 0;

  while(a <10)
    {
      // Put something on the stack
      stack_push(&s, a);
  
      // The stack should not be empty
      ASSERT_FALSE(stack_empty(&s));
      a++;
    }
  a--; //Bring the value back down to 9
  while(a>=0)
    {
      // Pop the element off the stack.
      ASSERT_EQ(a, stack_pop(&s));
      a--;
    }
  
  // The stack should be empty now
  ASSERT_TRUE(stack_empty(&s));
  
  // Clean up
  stack_destroy(&s);
}



void* insertnum(void* p)
{
  my_stack_t* s = (my_stack_t*)p;
  stack_push(s, 3);
  stack_push(s, 3);
  stack_push(s, 3);
  stack_push(s, 3);
  stack_push(s, 3);
  return NULL;
}

void* insertnumalt(void* p)
{
  my_stack_t* s = (my_stack_t*)p;
  stack_push(s, 2);
  stack_push(s, 4);
  stack_push(s, 6);
  stack_push(s, 8);
  stack_push(s, 10);
  return NULL;
}

//Checks Invar 3: If a thread pushes value A and then pushes value B, and no other thread pushes these specific values, A must not be popped from the stack before popping B.
TEST(StackTest, invar3)
{
  my_stack_t s;
  stack_init(&s);

  pthread_t threads[2];

  pthread_create(&threads[0], NULL, insertnum, &s);
  pthread_create(&threads[1], NULL, insertnumalt, &s);
  int cur = 10;
  while(!stack_empty(&s))
    {
      int l = stack_pop(&s);
      if(l!=3)
        {
          ASSERT_EQ(l, cur);
          cur = cur-2;
        }
      printf("%d\n", l);
    }
}

//The below is code given from Charlie Curtsinger for testing invarient 1/2

#define PUSH_TEST_THREADS 4
#define PUSH_TESTS 1000000
#define TEST_VALUE_LIMIT 100

void* pusher(void* p) {
  // Treat the thread argument as a stack pointer
  my_stack_t* s = (my_stack_t*)p;
  
  // Use this array to keep track of how many times we push each value
  int* value_counts = (int*)calloc(TEST_VALUE_LIMIT, sizeof(int));
  srand(time(0)); 
  // Push some random values
  for(int i=0; i<PUSH_TESTS; i++) {
    int value = rand() % TEST_VALUE_LIMIT;
    stack_push(s, value);
    value_counts[value]++;
  }

  // Send the value counts array back
  return value_counts;
}

TEST(StackTest, ParallelPush) {
  // Create a stack
  my_stack_t s;
  stack_init(&s);

  // Create threads to push to the stack
  pthread_t threads[PUSH_TEST_THREADS];
  for(int i=0; i<PUSH_TEST_THREADS; i++) {
    if(pthread_create(&threads[i], NULL, pusher, &s)) {
      perror("pthread_create failed");
      exit(2);
    }
  }

  int total_value_counts[TEST_VALUE_LIMIT];
  memset(total_value_counts, 0, sizeof(int) * TEST_VALUE_LIMIT);

  // Join with pusher threads
  for(int i=0; i<PUSH_TEST_THREADS; i++) {
	void* result;
    if(pthread_join(threads[i], &result)) {
      perror("pthread_join failed");
      exit(2);
    }
    
    // Treat the return from the thread as a value_counts array
    int* thread_value_counts = (int*)result;
    
    // Add thread value counts to the total value counts array
    for(int value=0; value<TEST_VALUE_LIMIT; value++) {
	  total_value_counts[value] += thread_value_counts[value];
	}
  }
  
  // Pop everything from the stack and decrement the popped value's count
  int value;
  while((value = stack_pop(&s)) != -1) {
	total_value_counts[value]--;
  }
  
  // If everything worked, all values in total_value_counts should be zero
  for(int i=0; i<TEST_VALUE_LIMIT; i++) {
    printf("i = %d is %d\n", i, total_value_counts[i]);
    ASSERT_EQ(0, total_value_counts[i]);
  }
}



// TEST(StackTest, ParallelPush11) {
//   // Create a stack
//   my_stack_t s;
//   stack_init(&s);

//   // Create threads to push to the stack
//   pthread_t threads[PUSH_TEST_THREADS];
//   for(int i=0; i<PUSH_TEST_THREADS; i++) {
//     if(pthread_create(&threads[i], NULL, pusher, &s)) {
//       perror("pthread_create failed");
//       exit(2);
//     }
//   }

//   int total_value_counts[TEST_VALUE_LIMIT];
//   memset(total_value_counts, 0, sizeof(int) * TEST_VALUE_LIMIT);

//   // Join with pusher threads
//   for(int i=0; i<PUSH_TEST_THREADS; i++) {
// 	void* result;
//     if(pthread_join(threads[i], &result)) {
//       perror("pthread_join failed");
//       exit(2);
//     }
    
//     // Treat the return from the thread as a value_counts array
//     int* thread_value_counts = (int*)result;
    
//     // Add thread value counts to the total value counts array
//     for(int value=0; value<TEST_VALUE_LIMIT; value++) {
// 	  total_value_counts[value] += thread_value_counts[value];
// 	}
//   }
  
//   // Pop everything from the stack and decrement the popped value's count
//   int value;
//   while((value = stack_pop(&s)) != -1) {
// 	total_value_counts[value]--;
//   }
  
//   // If everything worked, all values in total_value_counts should be zero
//   for(int i=0; i<TEST_VALUE_LIMIT; i++) {
// 	ASSERT_EQ(0, total_value_counts[i]);
//   }
// }



// // typedef struct insert_args {
// //   my_stack_t* stack;
// //   int val;
// //   int times;
// // } insert_args_t;

// // typedef struct insert_ret_args {
// //   int numthree;
// //   int numfour;
// // } insert_ret_args_t;

// // // Pass in stack, value of node, and # of times
// // void* insert(void* p) {
// //   insert_args_t* args = (insert_args_t*) p;
// //   int n = args->val;
// //   int cur = 0;
// //   n = 3;
// //   while(cur < 10) {
// //     printf("We insert %d\n", n);
// //     stack_push(args->stack, n);
// //     cur++;
// //   }

// //   return NULL;
// // }

// // void* pop(void* p) {
// //   insert_args_t* args = (insert_args_t*) p;
// //   int cur = 0;
// //   int threes = 0;
// //   int fours = 0;
// //   while(cur < 10) {
// //     int k = stack_pop(args->stack);
// //     if(k == 3)
// //       {
// //         threes++;
// //       }
// //     else if(k == 4)
// //       {
// //         fours++;
// //       }
// //     else if(k==-1)
// //       {
// //         printf("hi\n");
// //       }
// //     else
// //       {
// //         //printf("k is %d\n", k);
 
// //         //ASSERT_EQ(1, 2); //If our popped node isn't 3 or 4, we know something is wrong
// //       }
// //     cur++;
// //   }

// //   insert_ret_args_t* l = (insert_ret_args_t *)malloc(sizeof(insert_ret_args_t  ));
// //   l->numthree = threes;
// //   printf("Our threes are: %d\n", threes);
// //   l->numfour = fours;
  
// //   return (void *)l;
// // }

// // // Invariant 1 Test 2
// // TEST(StackTest, invar12) {
// //   // Create a stack
// //   my_stack_t s;
// //   stack_init(&s);

// //   pthread_t p1, p2;

// //   insert_args_t* p1_insert_args = (insert_args_t*) malloc(sizeof(insert_args_t));
// //   p1_insert_args->stack = &s;
// //   p1_insert_args->val = 3;
// //   p1_insert_args->times = 50;
// //   pthread_create(&p1, NULL, insert, &p1_insert_args);

// //   insert_args_t* p2_insert_args = (insert_args_t*) malloc(sizeof(insert_args_t));
// //   p2_insert_args->stack = &s;
// //   p2_insert_args->val = 4;
// //   p2_insert_args->times = 50;
// //   pthread_create(&p2, NULL, insert, &p2_insert_args);

// //   insert_ret_args_t* p1stuff;
// //   if(pthread_join(p1,NULL) != 0)
// //     {
// //       perror("Failed to join p1");
// //       exit(2);
// //     }
  
// //   insert_ret_args_t* p2stuff;
// //   if(pthread_join(p2,NULL) != 0)
// //     {
// //       perror("Failed to join p2");
// //       exit(2);
// //     }


// //   pthread_create(&p1, NULL, pop, &p1_insert_args);
// //   pthread_create(&p2, NULL, pop, &p2_insert_args);

// //   if(pthread_join(p1,(void**) &p1stuff) != 0)
// //     {
// //       perror("Failed to join p1");
// //       exit(2);
// //     }
  
// //   if(pthread_join(p2,(void**) &p2stuff) != 0)
// //     {
// //       perror("Failed to join p2");
// //       exit(2);
// //     }
  
// //   printf("Joined\n");
// //   printf("%d\n", p1stuff->numthree);
// //   int threes = p1stuff->numthree + p2stuff->numthree;
//   int fours = p1stuff->numfour + p2stuff->numfour;

//   printf("Our threes are %d\n", threes);
  
//   ASSERT_EQ(threes, 50);
//   ASSERT_EQ(fours, 50);
  
// }
