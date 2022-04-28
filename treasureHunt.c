#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <assert.h>
#include "uthread.h"
#include "queue.h"
#include "disk.h"

queue_t pending_read_queue;
int readsLeft;
volatile int nextBlock;
int indexInt = 0;
int indexIntTwo = 0;

void interrupt_service_routine()
{
  void *val;
  void *arg;
  void (*callback)(void *, void *);
  queue_dequeue(pending_read_queue, &val, &arg, &callback);
  callback(val, arg);
}

void printSum(void *resultv, void *countv)
{
  nextBlock = *((int *)resultv);
  printf("%d\n", nextBlock);
  exit(EXIT_SUCCESS);
}

void handleOtherReads(void *resultv, void *countv)
{
  nextBlock = *((int *)resultv);
  //readsLeft == *((int *) countv);
  //printf("Next Block: %d\n", nextBlock);
  //printf("Reads Left: %d\n", readsLeft);

  if(readsLeft != 1){
    disk_schedule_read(&nextBlock, nextBlock);
    queue_enqueue(pending_read_queue, &nextBlock, readsLeft--, handleOtherReads);
  }else {
    disk_schedule_read(&nextBlock, nextBlock);
    queue_enqueue(pending_read_queue, &nextBlock, NULL, printSum);
  }
}

void handleFirstRead(void *resultv, void *countv)
{
  readsLeft = *((int *)countv);
  nextBlock = *((int *)resultv);

  //printf("Next Block: - a %d\n", nextBlock);
  //printf("Reads Left: - a %d\n", readsLeft);

  //int *holder = malloc(indexInt * sizeof(int));
  disk_schedule_read(&nextBlock, nextBlock);
  queue_enqueue(pending_read_queue, &nextBlock, readsLeft--, handleOtherReads);
  indexIntTwo++;

  // Start the Hunt
  // for (int blockno = 0; blockno < readsLeft; blockno++)
  // {
  //   // printf("Entering the loop with index: %d\n", blockno);
  //   if (blockno == readsLeft - 1)
  //   {
  //     queue_enqueue(pending_read_queue, &holder[blockno], NULL, printSum);
  //   }
  //   else
  //   {
  //     queue_enqueue(pending_read_queue, &holder[blockno], NULL, handleOtherReads);
  //   }
  //   disk_schedule_read(&holder[blockno], nextBlock);
  // }
}

int main(int argc, char **argv)
{

  // Command Line Arguments
  static char *usage = "usage: treasureHunt starting_block_number";
  int starting_block_number;
  char *endptr;
  if (argc == 2)
    starting_block_number = strtol(argv[1], &endptr, 10);
  if (argc != 2 || *endptr != 0)
  {
    printf("argument error - %s \n", usage);
    return EXIT_FAILURE;
  }

  // Initialize
  uthread_init(1);
  disk_start(interrupt_service_routine);
  pending_read_queue = queue_create();
  int holderInt = 0;
  indexInt = starting_block_number;

  disk_schedule_read(&holderInt, starting_block_number);
  queue_enqueue(pending_read_queue, &holderInt, &holderInt, handleFirstRead);

  while (1); // infinite loop so that main doesn't return before hunt completes
}
