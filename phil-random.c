#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define MAX_THINKING_TIME 250

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf(S, ##__VA_ARGS__)
#else
#define VERBOSE_PRINT(S, ...) ((void)0) // do nothing
#endif

typedef struct fork
{
    uthread_mutex_t lock;
    uthread_cond_t forfree;
    long free;
} fork_t;

int num_phils, num_meals;
fork_t *forks;

void deep_thoughts()
{
    usleep(random() % MAX_THINKING_TIME);
}

void initfork(int i)
{
    forks[i].lock = uthread_mutex_create();
    forks[i].forfree = uthread_cond_create(forks[i].lock);
    forks[i].free = 1;
}

long getfork(long i)
{
    uthread_mutex_lock(forks[i].lock);

    // printf("reached get fork\n");
    if (forks[i].free)
    {
        forks[i].free = 0;
    }
    else
    {
        uthread_cond_wait(forks[i].forfree);
    }

    uthread_mutex_unlock(forks[i].lock);

    return 1;
}

void putfork(long i)
{
    /* TO BE IMPLEMENTED BY THE STUDENTS. */
    // printf("reached get fork inner\n");
    uthread_mutex_lock(forks[i].lock);
    forks[i].free = 1;
    uthread_cond_signal(forks[i].forfree);
    uthread_mutex_unlock(forks[i].lock);
}

int leftfork(long i)
{
    return i;
}

int rightfork(long i)
{
    return (i + 1) % num_phils;
}

void *phil_thread(void *arg)
{
    uintptr_t id = (uintptr_t)arg;
    int meals = 0;

    while (meals < num_meals)
    {
        /* TO BE IMPLEMENTED BY THE STUDENTS. */
        deep_thoughts();
        printf("P%d deep thoughts\n", *(int *)id + 1);

        if (random() % 2 == 0)
        {
            // lets call this case right fork first
            //  uthread_mutex_lock(forks[*(int *)id].lock);
            getfork(rightfork(*(int *)id));
            printf("P%d gets right fork\n", *(int *)id + 1);
            // uthread_mutex_unlock(forks[*(int *)id].lock);

            deep_thoughts();
            printf("P%d deep thoughts\n", *(int *)id + 1);

            // uthread_mutex_lock(forks[*(int *)id].lock);
            getfork(leftfork(*(int *)id));
            printf("P%d gets left fork\n", *(int *)id + 1);
            // uthread_mutex_unlock(forks[*(int *)id].lock);

            deep_thoughts();
            printf("P%d deep thoughts\n", *(int *)id + 1);
        }
        else
        {
            // lets call this left fork first
            //  uthread_mutex_lock(forks[*(int *)id].lock);
            getfork(leftfork(*(int *)id));
            printf("P%d gets left fork\n", *(int *)id + 1);
            // uthread_mutex_unlock(forks[*(int *)id].lock);

            deep_thoughts();
            printf("P%d deep thoughts\n", *(int *)id + 1);

            // uthread_mutex_lock(forks[*(int *)id].lock);
            getfork(rightfork(*(int *)id));
            printf("P%d gets right fork\n", *(int *)id + 1);
            // uthread_mutex_unlock(forks[*(int *)id].lock);

            deep_thoughts();
            printf("P%d deep thoughts\n", *(int *)id + 1);
        }

        // eating
        meals++;
        printf("P%d eats\n", *(int *)id + 1);

        deep_thoughts();
        printf("P%d deep thoughts\n", *(int *)id + 1);

        // uthread_mutex_lock(forks[*(int *)id].lock);
        putfork(leftfork(*(int *)id));
        putfork(rightfork(*(int *)id));
        printf("P%d puts down both forks\n", *(int *)id + 1);
        // uthread_mutex_unlock(forks[*(int *)id].lock);

        deep_thoughts();
        printf("P%d deep thoughts\n", *(int *)id + 1);
    }

    return 0;
}

int main(int argc, char **argv)
{
    uthread_t *p;
    uintptr_t i;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s num_philosophers num_meals\n", argv[0]);
        return 1;
    }

    num_phils = strtol(argv[1], 0, 0);
    num_meals = strtol(argv[2], 0, 0);

    forks = malloc(num_phils * sizeof(fork_t));
    p = malloc(num_phils * sizeof(uthread_t));

    uthread_init(num_phils);

    srandom(time(0));
    for (i = 0; i < num_phils; ++i)
    {
        initfork(i);
    }

    //=============================================================================

    /* TODO: Create num_phils threads, all calling phil_thread with a
     * different ID, and join all threads.
     */

    int holder[num_phils];
    // printf("reached thread creation");
    for (int i = 0; i < num_phils; ++i)
    {
        holder[i] = i;
    }

    for (int i = 0; i < num_phils; ++i)
    {
        printf("i value: %d\n", i);
        p[i] = uthread_create(phil_thread, &holder[i]);
    }

    // printf("Seg at Join");
    for (int i = 0; i < num_phils; i++)
    {
        uthread_join(p[i], NULL);
    }

    return 0;
}
