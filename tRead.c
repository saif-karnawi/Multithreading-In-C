#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/errno.h>
#include <assert.h>
#include "queue.h"
#include "disk.h"
#include "uthread.h"

queue_t pending_read_queue;
unsigned int sum = 0;

void interrupt_service_routine()
{
  void *val;
  void (*callback)(void *, void *);
  queue_dequeue(pending_read_queue, &val, NULL, NULL);
  uthread_unblock(val);
  // everytime we interrupt we gotta unblock the thread
}

void *read_block(void *blocknov)
{
  uthread_block();
  sum = sum + *((int *)blocknov);
  //printf("Current blocknov %d\n", *((int *)blocknov));
  return NULL;
}

int main(int argc, char **argv)
{
  // Command Line Arguments
  static char *usage = "usage: tRead num_blocks";
  int num_blocks;
  char *endptr;
  if (argc == 2)
    num_blocks = strtol(argv[1], &endptr, 10);
  if (argc != 2 || *endptr != 0)
  {
    printf("argument error - %s \n", usage);
    return EXIT_FAILURE;
  }

  // Initialize
  uthread_init(1);
  disk_start(interrupt_service_routine);
  pending_read_queue = queue_create();
  int *holder = malloc(num_blocks * sizeof(int));
  uthread_t *currThread = malloc(num_blocks * sizeof(uthread_t));

  // Sum Blocks
  for (int blockno = 0; blockno < num_blocks; blockno++){
    disk_schedule_read(&holder[blockno], blockno);
    currThread[blockno] = uthread_create(read_block, &holder[blockno]);
    queue_enqueue(pending_read_queue, currThread[blockno], NULL, NULL);
  }

  for (int i = 0; i < num_blocks; i++){
    uthread_join(currThread[i], NULL);
  }

  free(holder);
  free(currThread);
  printf("%d\n", sum);
}
