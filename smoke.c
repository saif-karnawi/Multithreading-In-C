#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "uthread.h"
#include "uthread_mutex_cond.h"

#define NUM_ITERATIONS 100

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf(S, ##__VA_ARGS__)
#else
#define VERBOSE_PRINT(S, ...) ((void)0) // do nothing
#endif
void *smokerWithTobacco(void *s_t);
void *smokerWithMatches(void *s_m);
void *smokerWithPapers(void *s_p);
void checker();
void *paperListener(void *arg);
void *tobaccoListener(void *arg);
void *matchListener(void *arg);

uthread_cond_t tSmokerHere;
uthread_cond_t mSmokerHere;
uthread_cond_t pSmokerHere;
int tobaccoHolder = 0;
int paperHolder = 0;
int matchHolder = 0;

struct Agent
{
  uthread_mutex_t mutex;
  uthread_cond_t match;
  uthread_cond_t paper;
  uthread_cond_t tobacco;
  uthread_cond_t smoke;
};

struct Agent *createAgent()
{
  struct Agent *agent = malloc(sizeof(struct Agent));
  agent->mutex = uthread_mutex_create();
  agent->paper = uthread_cond_create(agent->mutex);
  agent->match = uthread_cond_create(agent->mutex);
  agent->tobacco = uthread_cond_create(agent->mutex);
  agent->smoke = uthread_cond_create(agent->mutex);
  return agent;
}
//=============================================================================

//
// TODO
// You will probably need to add some procedures and struct etc.
//

//=============================================================================
struct Smoker
{
  uthread_mutex_t mutex;
  uthread_cond_t match;
  uthread_cond_t paper;
  uthread_cond_t tobacco;
  uthread_cond_t smoke;
};

struct Smoker *createSmoker()
{
  struct Smoker *smoker = malloc(sizeof(struct Smoker));
  smoker->mutex = uthread_mutex_create();
  smoker->paper = uthread_cond_create(smoker->mutex);
  smoker->match = uthread_cond_create(smoker->mutex);
  smoker->tobacco = uthread_cond_create(smoker->mutex);
  smoker->smoke = uthread_cond_create(smoker->mutex);
  return smoker;
}
//=============================================================================

// we need 3 methods, one for paper, smoker, and matches!
// We also need to make 3 methods that need to check if have enough to smoke!
//  - one for each smoker!
//  - or one method actually!

// The idea is to give the smoker with 1 ingrident to make the cigarette the 2 ingridents that the agent places

// And once that a cigratee is smoked, then the agent can place for ingridents for the smokers

//===============================================================================================================

/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */
enum Resource
{
  MATCH = 1,
  PAPER = 2,
  TOBACCO = 4
};
char *resource_name[] = {"", "match", "paper", "", "tobacco"};

// # of threads waiting for a signal. Used to ensure that the agent
// only signals once all other threads are ready.
int num_active_threads = 0;

int signal_count[5]; // # of times resource signalled
int smoke_count[5];  // # of times smoker with resource smoked

/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can modify it if you like, but be sure that all it does
 * is choose 2 random resources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void *agent(void *av)
{
  printf("Reached Agent\n");

  struct Agent *a = av;
  static const int choices[] = {MATCH | PAPER, MATCH | TOBACCO, PAPER | TOBACCO};
  static const int matching_smoker[] = {TOBACCO, PAPER, MATCH};

  srandom(time(NULL));

  uthread_mutex_lock(a->mutex);

  // Wait until all other threads are waiting for a signal
  // while (num_active_threads < 3)
  //   uthread_cond_wait(a->smoke);

  for (int i = 0; i < NUM_ITERATIONS; i++)
  {
    int r = random() % 6;
    switch (r)
    {
    case 0:
      tobaccoHolder = signal_count[TOBACCO];
      signal_count[TOBACCO]++;
      // if(tobaccoHolder < signal_count[TOBACCO]) uthread_cond_signal(tSmokerHere);
      printf("match available\n");
      uthread_cond_signal(a->match);
      printf("paper available\n");
      uthread_cond_signal(a->paper);
      break;
    case 1:
      paperHolder = signal_count[PAPER];
      signal_count[PAPER]++;
      // if(paperHolder < signal_count[PAPER]) uthread_cond_signal(pSmokerHere);
      printf("match available\n");
      uthread_cond_signal(a->match);
      printf("tobacco available\n");
      uthread_cond_signal(a->tobacco);
      break;
    case 2:
      matchHolder = signal_count[MATCH];
      signal_count[MATCH]++;
      // if(matchHolder < signal_count[MATCH]) uthread_cond_signal(mSmokerHere);
      printf("paper available\n");
      uthread_cond_signal(a->paper);
      printf("tobacco available\n");
      uthread_cond_signal(a->tobacco);
      break;
    case 3:
      tobaccoHolder = signal_count[TOBACCO];
      signal_count[TOBACCO]++;
      // if(tobaccoHolder < signal_count[TOBACCO]) uthread_cond_signal(tSmokerHere);
      printf("paper available\n");
      uthread_cond_signal(a->paper);
      printf("match available\n");
      uthread_cond_signal(a->match);
      break;
    case 4:
      paperHolder = signal_count[PAPER];
      signal_count[PAPER]++;
      // if(paperHolder < signal_count[PAPER]) uthread_cond_signal(pSmokerHere);
      printf("tobacco available\n");
      uthread_cond_signal(a->tobacco);
      printf("match available\n");
      uthread_cond_signal(a->match);
      break;
    case 5:
      matchHolder = signal_count[MATCH];
      signal_count[MATCH]++;
      // if(matchHolder < signal_count[MATCH]) uthread_cond_signal(mSmokerHere);
      printf("tobacco available\n");
      uthread_cond_signal(a->tobacco);
      printf("paper available\n");
      uthread_cond_signal(a->paper);
      break;
    }
    VERBOSE_PRINT("agent is waiting for smoker to smoke\n");
    uthread_cond_wait(a->smoke);
  }

  uthread_mutex_unlock(a->mutex);
  return NULL;
}

//=============================================================================

void *smokerWithTobacco(void *s_t)
{
  printf("Reached T Smoker\n");
  struct Agent *smoker = s_t;
  uthread_mutex_lock(smoker->mutex);

  while (1)
  {
    if (smoke_count[MATCH] + smoke_count[PAPER] + smoke_count[TOBACCO] != NUM_ITERATIONS)
    {
      uthread_cond_wait(tSmokerHere);
      printf("Tobacco smoker is smoking.\n");
      if (smoke_count[MATCH] + smoke_count[PAPER] + smoke_count[TOBACCO] != NUM_ITERATIONS)
      {
        smoke_count[TOBACCO]++;
      }

      uthread_cond_signal(smoker->smoke);
    }
    else
    {
      uthread_cond_broadcast(smoker->tobacco);
      uthread_cond_broadcast(smoker->paper);
      uthread_cond_broadcast(smoker->match);

      uthread_cond_broadcast(smoker->smoke);
      uthread_cond_broadcast(mSmokerHere);
      uthread_cond_broadcast(tSmokerHere);
      uthread_cond_broadcast(pSmokerHere);
      break;
    }
  }
  uthread_mutex_unlock(smoker->mutex);
  return NULL;
}

void *smokerWithMatches(void *s_m)
{
  printf("Reached M Smoker\n");
  struct Agent *smoker = s_m;
  uthread_mutex_lock(smoker->mutex);

  while (1)
  {
    // checker();
    if (smoke_count[MATCH] + smoke_count[PAPER] + smoke_count[TOBACCO] != NUM_ITERATIONS)
    {
      uthread_cond_wait(mSmokerHere);
      printf("Match smoker is smoking.\n");
      if (smoke_count[MATCH] + smoke_count[PAPER] + smoke_count[TOBACCO] != NUM_ITERATIONS)
      {
        smoke_count[MATCH]++;
      }

      uthread_cond_signal(smoker->smoke);
    }
    else
    {
      uthread_cond_broadcast(smoker->tobacco);
      uthread_cond_broadcast(smoker->paper);
      uthread_cond_broadcast(smoker->match);
      uthread_cond_broadcast(mSmokerHere);
      uthread_cond_broadcast(tSmokerHere);
      uthread_cond_broadcast(pSmokerHere);

      uthread_cond_broadcast(smoker->smoke);
      break;
    }
  }

  uthread_mutex_unlock(smoker->mutex);
  return NULL;
}

void *smokerWithPapers(void *s_p)
{
  printf("Reached P Smoker\n");
  struct Agent *smoker = s_p;
  uthread_mutex_lock(smoker->mutex);

  while (1)
  {
    // checker();
    if (smoke_count[MATCH] + smoke_count[PAPER] + smoke_count[TOBACCO] != NUM_ITERATIONS)
    {
      uthread_cond_wait(pSmokerHere);
      // include a middle method to check this idea
      printf("Paper smoker is smoking.\n");
      if (smoke_count[MATCH] + smoke_count[PAPER] + smoke_count[TOBACCO] != NUM_ITERATIONS)
      {
        smoke_count[PAPER]++;
      }
      uthread_cond_signal(smoker->smoke);
      // if(smoke_count[MATCH] + smoke_count[PAPER] + smoke_count[TOBACCO] == NUM_ITERATIONS){
      //   uthread_cond_broadcast(smoker->tobacco);
      //   uthread_cond_broadcast(smoker->paper);
      //   uthread_cond_broadcast(smoker->match);
    }
    else
    {
      uthread_cond_broadcast(smoker->tobacco);
      uthread_cond_broadcast(smoker->paper);
      uthread_cond_broadcast(smoker->match);
      uthread_cond_broadcast(smoker->smoke);
      uthread_cond_broadcast(mSmokerHere);
      uthread_cond_broadcast(tSmokerHere);
      uthread_cond_broadcast(pSmokerHere);
      break;
    }
    // }
    // break;
  }
  uthread_mutex_unlock(smoker->mutex);
  return NULL;
}

//=============================================================================

void *tobaccosListener(void *arg)
{
  printf("Reached Tobacco Checker\n");
  struct Agent *smoker = arg;
  uthread_mutex_lock(smoker->mutex);
  while (1)
  {
    if (smoke_count[MATCH] + smoke_count[PAPER] + smoke_count[TOBACCO] != NUM_ITERATIONS)
    {
      printf("Reached Tobacco Checker Inner\n");
      uthread_cond_wait(smoker->tobacco);
      printf("Reached Tobacco Checker Inner 2\n");
      checker();
    }
    else
    {
      uthread_cond_broadcast(smoker->smoke);
      break;
    }
  }
  uthread_mutex_unlock(smoker->mutex);
  return NULL;
}

void *papersListener(void *arg)
{

  printf("Reached Paper Checker\n");
  struct Agent *smoker = arg;
  uthread_mutex_lock(smoker->mutex);
  while (1)
  {

    printf("Reached Paper Checker Inner\n");
    if (smoke_count[MATCH] + smoke_count[PAPER] + smoke_count[TOBACCO] != NUM_ITERATIONS)
    {
      uthread_cond_wait(smoker->paper);
      printf("Reached Paper Checker Inner v2\n");
      checker();
    }
    else
    {
      uthread_cond_broadcast(smoker->smoke);

      break;
    }
  }

  uthread_mutex_unlock(smoker->mutex);
  return NULL;
}

void *matchesListener(void *arg)
{
  printf("Reached Match Checker\n");

  struct Agent *smoker = arg;
  uthread_mutex_lock(smoker->mutex);
  while (1)
  {
    if (smoke_count[MATCH] + smoke_count[PAPER] + smoke_count[TOBACCO] != NUM_ITERATIONS)
    {
      printf("Reached Match Checker Inner\n");
      uthread_cond_wait(smoker->match);
      printf("Reached Match Checker Inner v2\n");
      checker();
    }
    else
    {
      uthread_cond_broadcast(smoker->smoke);
      break;
    }
  }
  uthread_mutex_unlock(smoker->mutex);
  return NULL;
}

//=============================================================================

void checker()
{
  printf("Reached Checker\n");

  printf("T Holder:%d\n", tobaccoHolder);
  printf("P Holder:%d\n", paperHolder);
  printf("M Holder:%d\n", matchHolder);
  printf("T Signal:%d\n", signal_count[TOBACCO]);
  printf("P Signal:%d\n", signal_count[PAPER]);
  printf("M Signal:%d\n", signal_count[MATCH]);

  if (tobaccoHolder < signal_count[TOBACCO])
  {
    printf("1\n");
    tobaccoHolder = signal_count[TOBACCO] * 2;
    uthread_cond_signal(tSmokerHere);
  }
  else if (paperHolder < signal_count[PAPER])
  {
    printf("2\n");
    paperHolder = signal_count[PAPER] * 2;
    uthread_cond_signal(pSmokerHere);
  }
  else if (matchHolder < signal_count[MATCH])
  {
    printf("3\n");
    matchHolder = signal_count[MATCH] * 2;
    uthread_cond_signal(mSmokerHere);
  }
  else
  {
    return;
  }

  // uthread_mutex_unlock(smoker->mutex);
}

//=============================================================================

int main(int argc, char **argv)
{
  struct Agent *a = createAgent();
  uthread_t agent_thread;
  struct Smoker *s_t = createSmoker();
  uthread_t smokerTobacco_thread;
  struct Smoker *s_m = createSmoker();
  uthread_t smokerMatches_thread;
  struct Smoker *s_p = createSmoker();
  uthread_t smokePaper_thread;
  uthread_t paperListener;
  uthread_t tobaccoListener;
  uthread_t matchListener;

  uthread_init(20);

  // num_active_threads = num_active_threads + 3;

  tSmokerHere = uthread_cond_create(a->mutex);
  pSmokerHere = uthread_cond_create(a->mutex);
  mSmokerHere = uthread_cond_create(a->mutex);
  // checker_thread = uthread_create(checker, a);

  paperListener = uthread_create(papersListener, a);
  tobaccoListener = uthread_create(tobaccosListener, a);
  matchListener = uthread_create(matchesListener, a);

  smokerTobacco_thread = uthread_create(smokerWithTobacco, a);
  smokerMatches_thread = uthread_create(smokerWithPapers, a);
  smokePaper_thread = uthread_create(smokerWithMatches, a);
  agent_thread = uthread_create(agent, a);

  uthread_join(agent_thread, NULL);
  printf("Agent finished\n");
  printf("Total Smoke Count: %d\n", smoke_count[MATCH] + smoke_count[PAPER] + smoke_count[TOBACCO]);

  uthread_join(paperListener, NULL);
  printf("PL finished\n");

  uthread_join(tobaccoListener, NULL);
  printf("TL finished\n");

  uthread_join(matchListener, NULL);
  printf("ML finished\n");

  uthread_join(smokerTobacco_thread, NULL);
  printf("ST IS DONE\n");
  uthread_join(smokerMatches_thread, NULL);
  printf("SM IS DONE\n");

  uthread_join(smokePaper_thread, NULL);

  printf("SP IS DONE\n");

  uthread_cond_destroy(tSmokerHere);
  uthread_cond_destroy(mSmokerHere);
  uthread_cond_destroy(pSmokerHere);

  assert(signal_count[MATCH] == smoke_count[MATCH]);
  assert(signal_count[PAPER] == smoke_count[PAPER]);
  assert(signal_count[TOBACCO] == smoke_count[TOBACCO]);
  assert(smoke_count[MATCH] + smoke_count[PAPER] + smoke_count[TOBACCO] == NUM_ITERATIONS);

  printf("Smoke counts: %d matches, %d paper, %d tobacco\n",
         smoke_count[MATCH], smoke_count[PAPER], smoke_count[TOBACCO]);
           printf("Total Smoke Count: %d\n", smoke_count[MATCH] + smoke_count[PAPER] + smoke_count[TOBACCO]);


  return 0;
}
