#include <gtest/gtest.h>

#include "queue.hh"

/****** Queue Invariants ******/

// TODO: Write invariants here. See stack invariants for reference.

/****** Begin Tests ******/

// Basic queue functionality
TEST(QueueTest, BasicQueueOps) {
  my_queue_t q;
  queue_init(&q);
  
  // Make sure the queue is empty
  ASSERT_TRUE(queue_empty(&q));
  
  // Make sure taking from the queue returns -1
  ASSERT_EQ(-1, queue_take(&q));
  
  // Add some items to the queue
  queue_put(&q, 1);
  queue_put(&q, 2);
  queue_put(&q, 3);
  
  // Make sure the queue is not empty
  ASSERT_FALSE(queue_empty(&q));
  
  // Take the values from the queue and check them
  ASSERT_EQ(1, queue_take(&q));
  ASSERT_EQ(2, queue_take(&q));
  ASSERT_EQ(3, queue_take(&q));
  
  // Make sure the queue is empty
  ASSERT_TRUE(queue_empty(&q));
  
  // Clean up
  queue_destroy(&q);
}

void* insertnum(void* p)
{
  my_queue_t* s = (my_queue_t*)p;
  queue_put(s, 3);
  queue_put(s, 3);
  queue_put(s, 3);
  queue_put(s, 3);
  queue_put(s, 3);
  queue_put(s, 3);
  queue_put(s, 3);
  return NULL;
}

void* insertnumalt(void* p)
{
  my_queue_t* s = (my_queue_t*)p;
  queue_put(s, 2);
  queue_put(s, 4);
  queue_put(s, 6);
  queue_put(s, 8);
  queue_put(s, 10);
  return NULL;
}

//Checks Invar 3: If a thread pushes value A and then pushes value B, and no other thread pushes these specific values, A must not be popped from the stack before popping B.
TEST(QueueTest, invar3)
{
  my_queue_t s;
  queue_init(&s);
  //sleep(1);

  pthread_t threads[2];

  pthread_create(&threads[0], NULL, insertnum, &s);
  pthread_create(&threads[1], NULL, insertnumalt, &s);
  int cur = 2;
  while(!queue_empty(&s))
    {
      int l = queue_take(&s);
      if(l!=3)
        {
          ASSERT_EQ(l, cur);
          cur = cur+2;
          printf("Our stuff is %d\n", l);

        }
    }
}
#define PUSH_TEST_THREADS 4
#define PUSH_TESTS 1000
#define TEST_VALUE_LIMIT 100

void* pusher(void* p) {
  // Treat the thread argument as a stack pointer
  my_queue_t* s = (my_queue_t*)p;
  
  // Use this array to keep track of how many times we push each value
  int* value_counts = (int*)calloc(TEST_VALUE_LIMIT, sizeof(int));

  // Push some random values
  for(int i=0; i<PUSH_TESTS; i++) {
    int value = rand() % TEST_VALUE_LIMIT;
    queue_put(s, value);
    value_counts[value]++;
  }
  //printf("What is wrong with this\n");
  // Send the value counts array back
  return value_counts;
}

TEST(QueueTest, ParallelPush) {
  printf("We hit parallelpush\n");
  
  // Create a stack
  my_queue_t s;
  queue_init(&s);

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
    //printf("We jooined already\n");
    // Treat the return from the thread as a value_counts array
    int* thread_value_counts = (int*)result;
    
    // Add thread value counts to the total value counts array
    for(int value=0; value<TEST_VALUE_LIMIT; value++) {
	  total_value_counts[value] += thread_value_counts[value];
	}
  }
  
  // Pop everything from the stack and decrement the popped value's count
  int value;
  while((value = queue_take(&s)) != -1) {
	total_value_counts[value]--;
  }
  
  // If everything worked, all values in total_value_counts should be zero
  for(int i=0; i<TEST_VALUE_LIMIT; i++) {
	ASSERT_EQ(0, total_value_counts[i]);
  }
}

