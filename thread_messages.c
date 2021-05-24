// thread_messages.c: demonstrate use of a shared global data
// structure by several threads. Accessing/modifying the global array
// requires coordinationg between threads using a mutex to ensure
// correct behavior.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <pthread.h>
#include <string.h>
#include <wait.h>


char **msg_arr = NULL;   // array of messages, allocated as long as thread count
int msg_index = 0;       // current available index to add a messages
pthread_mutex_t lock;

// Introduce a mutex that allows threads to lock/unlock a
// critical region; global so it is in scope for all functions

void *add_msg(void *null){
  // function run by threads, adds messages to the global array

  //  Protect the shared data structure msg_arr / msg_index with
  // a mutex to maintain its consistency
  pthread_mutex_lock(&lock);
  msg_arr[msg_index] = strdup("I'm a thread!");
  msg_index++;
  pthread_mutex_unlock(&lock);
  return NULL;
}

int main(int argc, char *argv[]){
  if(argc < 2){
    printf("usage: %s <nthreads>\n",argv[0]);
    return 1;
  }

  int nthreads = atoi(argv[1]);                // first argument is number of threads
  pthread_t threads[nthreads];                 // array to track thread data

  msg_arr = calloc(sizeof(char *), nthreads);  // allocate memory calloc() initializes to 0/NULL

  //Initialize mutex prior to its use elsewhere
   pthread_mutex_init(&lock, NULL);
  for(int i=0; i<nthreads; i++){               // start all "child" threads
    pthread_create(&threads[i], NULL,
                   add_msg, NULL);
  }

  for(int i=0; i<nthreads; i++){               // wait for all "child" threads to finish
    pthread_join(threads[i], NULL);
  }

   pthread_mutex_destroy(&lock);

  printf("nthreads:  %d\n", nthreads);         // report results
  printf("msg_index: %d\n", msg_index);

  for(int i=0; i<nthreads; i++){               // print msg_arr[] contents
    printf("msg_arr[%d]: %s\n",                // should be all non-null
           i, msg_arr[i]);
    if(msg_arr[i] != NULL){                    // free strings allocated via strdup()
      free(msg_arr[i]);
    }
  }
  free(msg_arr);                               // free msg_arr[]

  return 0;
}
