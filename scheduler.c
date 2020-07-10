#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include "scheduler.h"

#include <assert.h>
#include <curses.h>
#include <ucontext.h>

#include "util.h"

// This is an upper limit on the number of tasks we can create.
#define MAX_TASKS 128

// This is the size of each task's stack memory
#define STACK_SIZE 65536

//Macros for state
#define ASLEEP 0
#define RUNNING 1
#define EXITED 2
#define READY_TO_RUN 3
#define WAITING 4
#define BLOCKED 5
#define WAITING_FOR_INPUT 6

// This struct will hold the all the necessary information for each task
typedef struct task_info {
  // This field stores all the state required to switch back to this task
  ucontext_t context;
  
  // This field stores another context. This one is only used when the task
  // is exiting.
  ucontext_t exit_context;

  //Look at the macros for the number 
  int state;
  //The time the task will wake up in ms
  long wake_up_time;
  //The task this task is waiting on, NULL if not waiting
  struct task_info * dependant;
  //NULL = not waiting for user input, otherwise will store user input in here
  int user_input;
  
  // TODO: Add fields here so you can:
  //   a. Keep track of this task's state.
  //   b. If the task is sleeping, when should it wake up?
  //   c. If the task is waiting for another task, which task is it waiting for?
  //   d. Was the task blocked waiting for user input? Once you successfully
  //      read input, you will need to save it here so it can be returned.
} task_info_t;

int current = 0; //< The handle of the currently-executing task
int num_tasks = 1;    //< The number of tasks created so far
task_info_t tasks[MAX_TASKS]; //< Information for every task

void scheduler(int state)
{
  tasks[current].state = state;
  int count = 0;
  while(count < num_tasks){ //loops while at least one task has not exited
    count = 0;
    int i = 0;
    for(int g = 0; g < num_tasks; g++){
      
      if(tasks[g].state == EXITED) //checks to see which tasks have exited
        {
          count++;
        }
      //if a task is waiting for input and there is input, that task is switched to
      if(tasks[g].state == WAITING_FOR_INPUT) { 
        int ch = getch();
        if(ch != ERR) {
          tasks[g].user_input = ch;
          tasks[g].state = RUNNING;
          int temp = current;
          current = g;
          swapcontext(&tasks[temp].context, &tasks[g].context);
          return;
          
        }
      }
      //set a task to READY_TO_RUN if the task it was waiting for has ended
      if(tasks[g].dependant != NULL && (tasks[g].dependant)->state == EXITED){
        tasks[g].dependant = NULL;
        tasks[g].state = READY_TO_RUN;
      }
      //checks to see if a task needs to be woken up, and if so, swaps to it
      if(tasks[g].state == ASLEEP && tasks[g].wake_up_time <= time_ms()) {
        tasks[g].state = RUNNING;
        int temp = current;
        current = g;
        swapcontext(&tasks[temp].context, &tasks[g].context);
        return;
      }

      
    }
    //run through the tasks ready to run and switch to the first one found (put into a second loop in order to prioritize any tasks that need to be woken up)
    while(i<num_tasks)
      {      
        if(tasks[i].state == READY_TO_RUN)
          {
            tasks[i].state = RUNNING;
            int temp = current;
            current = i;
            swapcontext(&tasks[temp].context, &tasks[i].context);
            return;
          }
      i++;
    }
  sleep_ms(1);
  }
  return;
}


/**
 * Initialize the scheduler. Programs should call this before calling any other
 * functions in this file.
 */
void scheduler_init() {
  tasks[current].state = RUNNING;
  tasks[current].wake_up_time = 0;
  tasks[current].dependant = NULL;
  tasks[current].user_input = ERR; 
}


/**
 * This function will execute when a task's function returns. This allows you
 * to update scheduler states and start another task. This function is run
 * because of how the contexts are set up in the task_create function.
 */
void task_exit() {
  scheduler(EXITED);
}

/**
 * Create a new task and add it to the scheduler.
 *
 * \param handle  The handle for this task will be written to this location.
 * \param fn      The new task will run this function.
 */
void task_create(task_t* handle, task_fn_t fn) {
  // Claim an index for the new task
  int index = num_tasks;
  num_tasks++;
  
  // Set the task handle to this index, since task_t is just an int
  *handle = index;
 
  // We're going to make two contexts: one to run the task, and one that runs at the end of the task so we can clean up. Start with the second
  
  // First, duplicate the current context as a starting point
  getcontext(&tasks[index].exit_context);
  
  // Set up a stack for the exit context
  tasks[index].exit_context.uc_stack.ss_sp = malloc(STACK_SIZE);
  tasks[index].exit_context.uc_stack.ss_size = STACK_SIZE;
  
  // Set up a context to run when the task function returns. This should call task_exit.
  makecontext(&tasks[index].exit_context, task_exit, 0);
  
  // Now we start with the task's actual running context
  getcontext(&tasks[index].context);
  
  // Allocate a stack for the new task and add it to the context
  tasks[index].context.uc_stack.ss_sp = malloc(STACK_SIZE);
  tasks[index].context.uc_stack.ss_size = STACK_SIZE;
  
  // Now set the uc_link field, which sets things up so our task will go to the exit context when the task function finishes
  tasks[index].context.uc_link = &tasks[index].exit_context;
  
  // And finally, set up the context to execute the task function
  makecontext(&tasks[index].context, fn, 0);
  tasks[index].state = READY_TO_RUN;
  tasks[index].dependant = NULL;
}

/**
 * Wait for a task to finish. If the task has not yet finished, the scheduler should
 * suspend this task and wake it up later when the task specified by handle has exited.
 *
 * \param handle  This is the handle produced by task_create
 */
void task_wait(task_t handle) {
  tasks[current].dependant = &tasks[handle];
  scheduler(WAITING);
}

/**
 * The currently-executing task should sleep for a specified time. If that time is larger
 * than zero, the scheduler should suspend this task and run a different task until at least
 * ms milliseconds have elapsed.
 * 
 * \param ms  The number of milliseconds the task should sleep.
 */
void task_sleep(size_t ms) {
  // TODO: Block this task until the requested time has elapsed.
  // Hint: Record the time the task should wake up instead of the time left for it to sleep. The bookkeeping is easier this way.
  tasks[current].wake_up_time = time_ms() + ms;
  scheduler(ASLEEP);
  
  
}

/**
 * Read a character from user input. If no input is available, the task should
 * block until input becomes available. The scheduler should run a different
 * task while this task is blocked.
 *
 * \returns The read character code
 */
int task_readchar() {
  // TODO: Block this task until there is input available.
  // To check for input, call getch(). If it returns ERR, no input was available.
  // Otherwise, getch() will returns the character code that was read.
  scheduler(WAITING_FOR_INPUT);
  return tasks[current].user_input;
}
