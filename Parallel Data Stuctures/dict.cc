#include "dict.hh"

#include <stdlib.h>

void list_init(list_t* lis, char a)
{
  lis->root = NULL;
  lis->starting_char = a;
  pthread_mutex_init(&lis->lock, NULL);
        
}

// Initialize a dictionary
void dict_init(my_dict_t* dict) {
  pthread_mutex_init(&dict->lock, NULL);


  int l = 0;
  while(l<26)
    {
      dict->list[l] = (list_t*)malloc(sizeof(list_t));
      dict->list[l]->root = NULL;
      dict->list[l]->starting_char = 'a' + l;

      pthread_mutex_init(&dict->list[l]->lock, NULL);
      // printf("Hello there\n");
      l++;
    }

  l = 0;
  while(l<26)
    {
      dict->list[l+26] = (list_t*)malloc(sizeof(list_t));

      dict->list[l+26]->root = NULL;
      dict->list[l+26]->starting_char = 'A' + l;
      pthread_mutex_init(&dict->list[l+26]->lock, NULL);
      l++;
    }
  //printf("Hi\n");

}

void destroy_list(list_t* des)
{
  pthread_mutex_lock(&des->lock);

  node_t* temp = des->root;
  while(temp!=NULL)
    {
      node_t* l = temp;
      temp = temp->next;
      free(l);
    }
  pthread_mutex_unlock(&des->lock);

}

// Destroy a dictionary
void dict_destroy(my_dict_t* dict) {
  pthread_mutex_lock(&dict->lock);

  int a = 0;
  while(a<52)
    {
      destroy_list(dict->list[a]);
      a++;
    }
  pthread_mutex_unlock(&dict->lock);

}

// Set a value in a dictionary
void dict_set(my_dict_t* dict, const char* key, int value) {
  //Gets the first letter of the key, we use this as an index for storage
  char c = key[0];
  int a = -1;
  //Checks if the first letter of the key is an actual letter
  if( !((c<= 'z' && c>= 'a') || (c<= 'Z' && c>= 'A'))  )
    {
      //printf("Our stuff aren't in a place \n");
      return;
    }
  //Case where the first letter is lowercase
  else if(c<= 'z' && c>= 'a')
    {
      a = c - 'a';
    }
  //Case where the first letter is uppercase
  else
    {
      a = c - 'A'+26;
    }
  //printf("We put in something at index %d\n", a);
  //We make the actual node
  node_t* nod = (node_t*)malloc(sizeof(node_t));
  nod->val = value;
  nod->key = key;
  nod->next = NULL;
  //printf("We got to here\n");

  //Lock the list we're editing
  pthread_mutex_lock(&dict->list[a]->lock );

  //We find the correct place for it
  node_t* temp = dict->list[a]->root;

  //Case where there's nothing in the list
  if(temp == NULL)
    {
      dict->list[a]->root = nod;
    }
  else if(temp->key == key)
    {
      nod->next = temp->next;
      dict->list[a]->root = nod;
      free(temp);
    }
  else
    {
      //We keep looking until we find an empty space
      while(temp->next!=NULL)
        {
          //printf("We are in teh while\n");
          if(temp->next->key == key)
            {
              node_t* ss = temp->next;
              temp->next = nod;
              pthread_mutex_unlock(&dict->list[a]->lock);
              free(ss);
              return;
            }
          
          temp = temp->next;
        }
      //Insert our node into the empty space
      temp->next = nod;
    }
  //Unlocks the list

  pthread_mutex_unlock(&dict->list[a]->lock);
}

// Check if a dictionary contains a key
bool dict_contains(my_dict_t* dict, const char* key) {
  char c = key[0];
  int index = 0;
  //Checks if the first letter of the key is an actual letter
  if( !((c<= 'z' && c>= 'a') || (c<= 'Z' && c>= 'A'))  )
    {
      //printf("We return false\n");
      return false;
    }
  //Case where the first letter is lowercase
  else if(c<= 'z' && c>= 'a')
    {
      index = c - 'a';
    }
  //Case where the first letter is uppercase
  else
    {
      index = c - 'A' + 26;
    }
  //printf("We found our index: %d\n", index);
  pthread_mutex_lock(&dict->list[index]->lock);
  //printf("We found our lock\n");
  node_t* temp;
  if(dict->list[index]->root == NULL)
    {
      pthread_mutex_unlock(&dict->list[index]->lock);

      return false;
    }
  else
    temp = dict->list[index]->root;
  //printf("Our key is %s\n", temp->key);
  while(strcmp(temp->key, key)!=0) {
    if(temp->next == NULL) {
      pthread_mutex_unlock(&dict->list[index]->lock);

      return false;
    }
    temp = temp->next;
  }
  pthread_mutex_unlock(&dict->list[index]->lock);
  return true;
}

// Get a value in a dictionary
int dict_get(my_dict_t* dict, const char* key) {
  char c = key[0];
  int index = 0;
  //Checks if the first letter of the key is an actual letter
  if( !((c<= 'z' && c>= 'a') || (c<= 'Z' && c>= 'A'))  )
    {
      printf("key is not a letter");
      //exit(1);
      return -1;
    }
  //Case where the first letter is lowercase
  else if(c<= 'z' && c>= 'a')
    {
      index = c - 'a';
    }
  //Case where the first letter is uppercase
  else
    {
      index = c - 'A' + 26;
    }
  pthread_mutex_lock(&dict->list[index]->lock);
  if(dict->list[index]->root == NULL)
    {
      pthread_mutex_unlock(&dict->list[index]->lock);

      return -1;
    }
  node_t* temp = dict->list[index]->root;
  // printf("We are eeeeee compared %s with %s \n", temp->key, key);

  while(strcmp(temp->key, key)!=0) {
    //printf("We compared %s with %s \n", temp->key, key);
    if(temp->next == NULL) {
      pthread_mutex_unlock(&dict->list[index]->lock);

      return -1;
    }
    temp = temp->next;
  }
  int element = temp->val;
  pthread_mutex_unlock(&dict->list[index]->lock);
  return element;
}

// Remove a value from a dictionary
void dict_remove(my_dict_t* dict, const char* key) {
    char c = key[0];
  int index = 0;
  //Checks if the first letter of the key is an actual letter
  if( !((c<= 'z' && c>= 'a') || (c<= 'Z' && c>= 'A'))  )
    {
      //printf("key is not a letter");
      return;
    }
  //Case where the first letter is lowercase
  else if(c<= 'z' && c>= 'a')
    {
      index = c - 'a';
    }
  //Case where the first letter is uppercase
  else
    {
      index = c - 'A' + 26;
    }
  pthread_mutex_lock(&dict->list[index]->lock);
  if(dict->list[index]->root == NULL)
    {
      printf("Dictionary does not contain requested key: %s\n", key);

      pthread_mutex_unlock(&dict->list[index]->lock);
      return;
    }
  node_t* temp = dict->list[index]->root;
  if(strcmp(temp->key, key)==0)
    {
      dict->list[index]->root = temp->next;
      free(temp);
        pthread_mutex_unlock(&dict->list[index]->lock);

      return;
    }
  while(strcmp(temp->next->key, key)!=0) {
    if(temp->next->next == NULL) {
      printf("Dictionary does not contain requested key: %s\n", key);
      return;
    }
    temp = temp->next;
  }
  //Now temp->next is the key
  node_t* l = temp->next;
  temp->next = temp->next->next;
  free(l);
  
  pthread_mutex_unlock(&dict->list[index]->lock);
}
