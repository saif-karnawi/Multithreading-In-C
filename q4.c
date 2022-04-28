#include <stdlib.h>
#include <stdio.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

uthread_t t0, t1, t2;

uthread_cond_t zero;
uthread_cond_t one;
uthread_cond_t two;
uthread_mutex_t mx;

int zeroHasGone;
int oneHasGone;
int twoHasGone;

void randomStall() {
  int i, r = random() >> 16;
  while (i++<r);
}

void* p0(void* v) {
  randomStall();

  uthread_mutex_lock(mx);

  printf("zero\n");
  zeroHasGone = 1; 
  uthread_cond_signal(zero);
  
  uthread_mutex_unlock(mx);

  return NULL;
}

void* p1(void* v) {



  uthread_mutex_lock(mx);

  if(zeroHasGone == 0) {
    uthread_cond_wait(zero);
  }

  randomStall();
  oneHasGone = 1;
  printf("one\n");
  uthread_cond_signal(one);

  uthread_mutex_unlock(mx);
  
  return NULL;
}

void* p2(void* v) {

  uthread_mutex_lock(mx);

  if(zeroHasGone == 0) {
    uthread_cond_wait(zero);
  } else if (oneHasGone == 0) {
    uthread_cond_wait(one);
  }
  


  randomStall();
  printf("two\n");

  uthread_mutex_unlock(mx);

  return NULL;
}

int main(int arg, char** arv) {
  
  uthread_init(4);
  
  mx = uthread_mutex_create();

  zero = uthread_cond_create(mx);
  one = uthread_cond_create(mx);
  two = uthread_cond_create(mx);

  t0 = uthread_create(p0, NULL);
  t1 = uthread_create(p1, NULL);
  t2 = uthread_create(p2, NULL);
  randomStall();
  uthread_join (t0, NULL);
  uthread_join (t1, NULL);
  uthread_join (t2, NULL);
  printf("three\n");
  printf("------\n");

}
