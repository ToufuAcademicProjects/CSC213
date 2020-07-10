#include <gtest/gtest.h>

#include "dict.hh"

/****** Dictionary Invariants ******/

// TODO: Write invariants here. See stack invariants for reference.

/****** Synchronization ******/

// Under what circumstances can accesses to your dictionary structure can proceed in parallel? Answer below.
// When the first letter of the key is different, we can run in parallel for all commands except for the destroy command.
// 
// 

// When will two accesses to your dictionary structure be ordered by synchronization? Answer below.
// When the first letters of the two keys are the same (a != A), the threads will never be able to run in parallel even though a different implementation might be able to do it. This is because we order our (key, val) pairs using the first letter of the key and have a lock for each letter.
// 
// 

/****** Begin Tests ******/

// Basic functionality for the dictionary
TEST(DictionaryTest, BasicDictionaryOps) {
  my_dict_t d;
  dict_init(&d);

  // Make sure the dictionary does not contain keys A, B, and C
  ASSERT_FALSE(dict_contains(&d, "A"));
  ASSERT_FALSE(dict_contains(&d, "B"));
  ASSERT_FALSE(dict_contains(&d, "C"));

  // Add some values
  dict_set(&d, "A", 1);
  dict_set(&d, "B", 2);
  dict_set(&d, "C", 3);
  // Make sure these values are contained in the dictionary
  ASSERT_TRUE(dict_contains(&d, "A"));
  ASSERT_TRUE(dict_contains(&d, "B"));
  ASSERT_TRUE(dict_contains(&d, "C"));

  // Make sure these values are in the dictionary
  ASSERT_EQ(1, dict_get(&d, "A"));
  ASSERT_EQ(2, dict_get(&d, "B"));
  ASSERT_EQ(3, dict_get(&d, "C"));

  // Set some new values
  dict_set(&d, "A", 10);
  dict_set(&d, "B", 20);
  dict_set(&d, "C", 30);

  // Make sure these values are contained in the dictionary
  ASSERT_TRUE(dict_contains(&d, "A"));
  ASSERT_TRUE(dict_contains(&d, "B"));
  ASSERT_TRUE(dict_contains(&d, "C"));

  // Make sure the new values are in the dictionary
  ASSERT_EQ(10, dict_get(&d, "A"));
  ASSERT_EQ(20, dict_get(&d, "B"));
  ASSERT_EQ(30, dict_get(&d, "C"));

  // Remove the values
  dict_remove(&d, "A");
  dict_remove(&d, "B");
  dict_remove(&d, "C");

  // Make sure these values are not contained in the dictionary
  ASSERT_FALSE(dict_contains(&d, "A"));
  ASSERT_FALSE(dict_contains(&d, "B"));
  ASSERT_FALSE(dict_contains(&d, "C"));

  // Make sure we get -1 for each value
  ASSERT_EQ(-1, dict_get(&d, "A"));
  ASSERT_EQ(-1, dict_get(&d, "B"));
  ASSERT_EQ(-1, dict_get(&d, "C"));

  // Clean up
  dict_destroy(&d);
}


//1. We can't have any (key, val) pair that we didn't insert in

//1.5 The size of our table will be the same as the number of unique keys inserted in

//2. Searching/Getting for a key will always return the val corresponding to the key in acccordance to the last insert.

//3. After removing a (key, val) pair we won't be able to find it anymore using the key

void* thread_helper(void* arg) {
  my_dict_t* d = (my_dict_t*) arg;

  //Insertion
  dict_set(d, "A", 10);
  dict_set(d, "B", 20);
  dict_set(d, "C", 30);
  
  return NULL;
}


TEST(DictionaryTest, Invar1and1point5)
{
  //First, initialize the dictionary
  my_dict_t d;
  dict_init(&d);

  pthread_t p1, p2;
  pthread_create(&p1, NULL, thread_helper, &d);
  pthread_create(&p2, NULL, thread_helper, &d);

  pthread_join(p1, NULL);
  pthread_join(p2, NULL);

  //Should be true  
  ASSERT_EQ(dict_get(&d, "A"), 10);
  ASSERT_EQ(dict_get(&d, "B"), 20);
  ASSERT_EQ(dict_get(&d, "C"), 30);

  //Should be false
  ASSERT_EQ(dict_get(&d, "D"), -1);
  ASSERT_EQ(dict_get(&d, "E"), -1);
  ASSERT_EQ(dict_get(&d, "F"), -1);
}
void* thread_helper2(void* arg) {
  my_dict_t* d = (my_dict_t*) arg;

  //Insertion
  dict_set(d, "A", 100);
  dict_set(d, "B", 200);
  dict_set(d, "C", 300);
  
  return NULL;
}

TEST(DictionaryTest, Invar2)
{
  //First, initialize the dictionary
  my_dict_t d;
  dict_init(&d);

  pthread_t p1, p2;
  pthread_create(&p1, NULL, thread_helper, &d);
  pthread_create(&p2, NULL, thread_helper, &d);

  pthread_join(p1, NULL);
  pthread_join(p2, NULL);

  pthread_t p3, p4;
  pthread_create(&p3, NULL, thread_helper2, &d);
  pthread_create(&p4, NULL, thread_helper2, &d);

  pthread_join(p3, NULL);
  pthread_join(p4, NULL);
  
  //Should be true  
  ASSERT_EQ(dict_get(&d, "A"), 100);
  ASSERT_EQ(dict_get(&d, "B"), 200);
  ASSERT_EQ(dict_get(&d, "C"), 300);

  //Should be false
  //ASSERT_EQ(dict_get(&d, "A"), 10);
  //ASSERT_EQ(dict_get(&d, "B"), 20);
  //ASSERT_EQ(dict_get(&d, "C"), 30);
}

void* thread_helper3(void* arg) {
  my_dict_t* d = (my_dict_t*) arg;

  //Insertion
  dict_remove(d, "A");
  dict_remove(d, "B");
  dict_remove(d, "C");
  
  return NULL;
}


TEST(DictionaryTest, Invar3)
{
  my_dict_t d;
  dict_init(&d);

  
  dict_set(&d, "A", 100);
  dict_set(&d, "B", 200);
  dict_set(&d, "C", 300);

  pthread_t p1, p2;
  pthread_create(&p1, NULL, thread_helper3, &d);
  pthread_create(&p2, NULL, thread_helper3, &d);

  pthread_join(p1, NULL);
  pthread_join(p2, NULL);

  ASSERT_FALSE(dict_contains(&d, "A"));
  ASSERT_FALSE(dict_contains(&d, "B"));
  ASSERT_FALSE(dict_contains(&d, "C"));
}
