#include "ui.h"
#include "client.h"
#include <form.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "socket.h"
#include <unistd.h>

// The height of the input field in the user interface
#define INPUT_HEIGHT 3

// The timeout for input
#define INPUT_TIMEOUT_MS 10

// The time for a warning to be displayed
#define WARNING_TIME 5

// The ncurses forms code is loosely based on the first example at http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/forms.html

// The fields array for the display form
FIELD* display_fields[2];

// The form that holds the display field
FORM* display_form;

// The fields array for the input form
FIELD* input_fields[2];

// The form that holds the input field
FORM* input_form;

// The handle for the UI thread
pthread_t ui_thread;

// A lock to protect the entire UI
pthread_mutexattr_t ui_lock_attr;
pthread_mutex_t ui_lock;

// The callback function to run on new input
static input_callback_t input_callback;

// When true, the UI should continue running
bool ui_running = false;

size_t time_ms() {
  struct timeval tv;
  if(gettimeofday(&tv, NULL) == -1) {
    perror("gettimeofday");
    exit(2);
  }
  
  // Convert timeval values to milliseconds
  return tv.tv_sec*1000 + tv.tv_usec/1000;
}


/**
 * Add a new message to the user interface's display pane.
 *
 * \param username  The username that should appear before the message. The UI
 *                  code will copy the string out of message, so it is safe to
 *                  reuse the memory pointed to by message after this function.
 *
 * \param message   The string that should be added to the display pane. As with
 *                  the username, the UI code will copy the string passed in.
 */
void ui_display(const char* username, const char* message) {
  // Lock the UI
  pthread_mutex_lock(&ui_lock);
  
  // Don't do anything if the UI is not running
  if(ui_running) {
    // Add a newline
    form_driver(display_form, REQ_NEW_LINE);
    
    // Display the username
    const char* c = username;
    while(*c != '\0') {
      form_driver(display_form, *c);
      c++;
    }
    form_driver(display_form, ':');
    form_driver(display_form, ' ');
    
    // Copy the message over to the display field
    c = message;
    while(*c != '\0') {
      form_driver(display_form, *c);
      c++;
    }
  } else {
    printf("%s: %s\n", username, message);
  }
  
  // Unlock the UI
  pthread_mutex_unlock(&ui_lock);
}


/**
 * Initialize the user interface and set up a callback function that should be
 * called every time there is a new message to send.
 *
 * \param callback  A function that should run every time there is new input.
 *                  The string passed to the callback should be copied if you
 *                  need to retain it after the callback function returns.
 */
void ui_init() {
  // Initialize curses
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  timeout(INPUT_TIMEOUT_MS);
  keypad(stdscr, TRUE);
  
  // Get the number of rows and columns in the terminal display
  int rows;
  int cols;
  getmaxyx(stdscr, rows, cols); // This uses a macro to modify rows and cols
  
  // Calculate the height of the display field
  int display_height = rows - INPUT_HEIGHT - 1;

  // Create the larger message display window
  // height, width, start row, start col, overflow buffer lines, buffers
  display_fields[0] = new_field(display_height, cols, 0, 0, 0, 0);
  display_fields[1] = NULL;
  
  // Create the input field
  input_fields[0] = new_field(INPUT_HEIGHT, cols, display_height + 1, 0, 0, 0);
  input_fields[1] = NULL;
  
  // Grow the display field buffer as needed
  field_opts_off(display_fields[0], O_STATIC);
  
  // Don't advance to the next field automatically when using the input field
  field_opts_off(input_fields[0], O_AUTOSKIP);
  
  // Turn off word wrap (nice, but causes other problems)
  field_opts_off(input_fields[0], O_WRAP);
  field_opts_off(display_fields[0], O_WRAP);
  
  // Create the forms
  display_form = new_form(display_fields);
  input_form = new_form(input_fields);
  
  // Display the forms
  post_form(display_form);
  post_form(input_form);
  refresh();
  
  // Draw a horizontal split
  for(int i=0; i<cols; i++) {
    mvprintw(display_height, i, "-");
  }
  
  // Update the display
  refresh();
  

 
  
  // Initialize the UI lock
  pthread_mutexattr_init(&ui_lock_attr);
  pthread_mutexattr_settype(&ui_lock_attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&ui_lock, &ui_lock_attr);
  
  // Running
  ui_running = true;
  ui_display("Game","Welcome to the Trivia Game");
  ui_display("Game", "Please wait for the first question");
}

/**
 * Run the main UI loop. This function will only return the UI is exiting.
 */
void ui_wait_for_input(int duration) {
  // Loop as long as the UI is running
  struct timespec start, cur;
  clock_gettime(CLOCK_MONOTONIC, &start);
  clock_gettime(CLOCK_MONOTONIC, &cur);
  int start_time = time_ms();
  int cur_time =  start_time;
  int warning_displayed = 0;

  while(ui_running && cur.tv_sec <= start.tv_sec+duration) {
    /*char buff[250];
    snprintf(buff, 250, "%lld", (long long) cur.tv_sec);
    ui_display("Time", buff); */

    if(warning_displayed == 0 && cur.tv_sec >= start.tv_sec+duration-WARNING_TIME)
      {
        //Display the warning
        ui_display("You", "suk");
        warning_displayed = 1;
      }



    
    // Get a character
    int ch = getch();
    
    // If there was no character, try again
    if(ch == -1) continue;
    
    // There was some character. Lock the UI
    pthread_mutex_lock(&ui_lock);
    
    // Handle input
    if(ch == KEY_BACKSPACE || ch == KEY_DC || ch == 127) {
      // Delete the last character when the user presses backspace
      form_driver(input_form, REQ_DEL_PREV);
      
    } else if(ch == KEY_ENTER || ch == '\n') {
      // When the user presses enter, report new input
      
      // Shift to the "next" field (same field) to update the buffer
      form_driver(input_form, REQ_NEXT_FIELD);
      
      // Get a pointer to the start of the input buffer
      char* buffer = field_buffer(input_fields[0], 0);
      
      // Get a pointer to the end of the input buffer
      char* buffer_end = buffer + strlen(buffer) - 1;
      
      // Seek backward until we find a non-space character in the buffer
      while(buffer_end[-1] == ' ' && buffer_end >= buffer) {
        buffer_end--;
      }
      
      // Compute the length of the input buffer
      int buffer_len = buffer_end - buffer;
      
      // If there's a message, handle it
      if(buffer_len > 0) {
        // Copy the message string out so it can be null-terminated
        char message[buffer_len+1];
        memcpy(message, buffer, buffer_len);
        message[buffer_len] = '\0';
      
        send_message(message);
      
        // Clear the input field, but only if the UI didn't exit
        if(ui_running) form_driver(input_form, REQ_CLR_FIELD);
        return;
      }
      
    } else {
      // Report normal input characters to the input field
      form_driver(input_form, ch);
    }
    if(ui_running) form_driver(input_form, REQ_CLR_FIELD);
    clock_gettime(CLOCK_MONOTONIC, &cur);
    // Unlock the UI
    pthread_mutex_unlock(&ui_lock);
  }
  ui_display("Game:", "You failed to enter an answer. Be better next time");
}


/**
 * Stop the user interface and clean up.
 */
void ui_exit() {
  // Block access to the UI
  pthread_mutex_lock(&ui_lock);
  
  // The UI is not running
  ui_running = false;
  
  // Clean up
  unpost_form(display_form);
  unpost_form(input_form);
  free_form(display_form);
  free_form(input_form);
  free_field(display_fields[0]);
  free_field(input_fields[0]);
  endwin();
  
  // Unlock the UI
  pthread_mutex_unlock(&ui_lock);
}
