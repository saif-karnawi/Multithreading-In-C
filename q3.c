#include <stdlib.h>
#include <stdio.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define NUM_THREADS 3
uthread_t threads[NUM_THREADS];
uthread_mutex_t mx;
int done;

void randomStall() {
  int i, r = random() >> 16;
  while (i++<r);
}

void waitForAllOtherThreads() {

  if(done != NUM_THREADS) {
    uthread_mutex_unlock(mx);
    uthread_block();
  } else {
    done--;
  }
    
}

void* p(void* v) {
  randomStall();
  
  uthread_mutex_lock(mx);

  printf("a\n");

  done++;

  waitForAllOtherThreads();


  printf("b\n");
  
  done--;

  if(done >= 0) {
    uthread_unblock(threads[done]);
  }

  return NULL;
}

int main(int arg, char** arv) {
  done = 0;
  uthread_init(4);
  mx = uthread_mutex_create();

  for (int i=0; i<NUM_THREADS; i++)
    threads[i] = uthread_create(p, NULL);
  for (int i=0; i<NUM_THREADS; i++)
    uthread_join (threads[i], NULL);
  printf("------\n");
}