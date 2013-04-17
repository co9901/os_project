/* Creates N threads, each of which sleeps a different, fixed
   duration, M times.  Records the wake-up order and verifies
   that it is valid. */

#include <stdio.h>
#include <string.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"
static void gogo(); //minsik
static void test_sleep (int thread_cnt, int iterations,int l);
static int porder[50000][2]; //minsik
static char threadname[50000][20];
static int porder_count=0; //minsik
static int p2order[50000];
static int time[50000][2];
static int time_count=0;
static int pri[1001];
static int end_count=0;


void
my_test_alarm_single (void) 
{
	test_sleep (3, 1,0);
}

	void
test_my_alarm_fairness (void) 
{
   test_sleep (200, 2,0);
}
void test_my_alarm_ef (void)
{
   test_sleep (200,2,1);
}

/* Information about the test. */
struct sleep_test 
{
	int64_t start;              /* Current time at start of test. */
	int iterations;             /* Number of iterations per thread. */

	/* Output. */
	struct lock output_lock;    /* Lock protecting output buffer. */
	int *output_pos;            /* Current position in output buffer. */
};

/* Information about an individual thread in the test. */
struct sleep_thread 
{
	struct sleep_test *test;     /* Info shared between all threads. */
	int id;                     /* Sleeper ID. */
	int duration;               /* Number of ticks to sleep. */
	int iterations;             /* Iterations counted so far. */
};

static void sleeper (void *);

/* Runs THREAD_CNT threads thread sleep ITERATIONS times each. */
	static void
test_sleep (int thread_cnt, int iterations,int l) 
{
	struct sleep_test test;
	struct sleep_thread *threads;
	int *output, *op;
	int product;
	int i;

	/* This test does not work with the MLFQS. */
	ASSERT (!thread_mlfqs);

	msg ("Creating %d threads to sleep %d times each.", thread_cnt, iterations);
	msg ("Thread 0 sleeps 10 ticks each time,");
	msg ("thread 1 sleeps 20 ticks each time, and so on.");
	msg ("If successful, product of iteration count and");
	msg ("sleep duration will appear in nondescending order.");

	/* Allocate memory. */
	threads = malloc (sizeof *threads * thread_cnt);
	output = malloc (sizeof *output * iterations * thread_cnt * 2);
	if (threads == NULL || output == NULL)
		PANIC ("couldn't allocate memory for test");

	/* Initialize test. */
	test.start = timer_ticks () + 100;
	test.iterations = iterations;
	lock_init (&test.output_lock);
	test.output_pos = output;

	/* Start threads. */
	ASSERT (output != NULL);

	for (i = 0; i < thread_cnt; i++)
	{
		struct sleep_thread *t = threads + i;
		char name[16];

	
		snprintf (name, sizeof name, "thread %d", i);
		thread_create (name, i%30, gogo, NULL);//프라이어리티알아서

	}

	/* Wait long enough for all the threads to finish. */
	timer_sleep (100 + thread_cnt * iterations * 200 + 100); //적당히쉬는시간

	/* Acquire the output lock in case some rogue thread is still
	   running. */
if(l==0){
	for(i=0;i<porder_count;i++)
	{
		if(threadname[i][0]=='t'){
		printf("%d %s\n",p2order[i],threadname[i]);}
	}

}
if(l==1)
{
	for(i=0;i<time_count;i++)
	{
		printf("%d\n",time[i][0]);
}
}  
	lock_acquire (&test.output_lock);
	lock_release (&test.output_lock);

	free (output);
	free (threads);
	thread_print_stats();
}
static void gogo()
{
	int t=0;
	while(t<=1000000) //적당히 루프 끝내기위해서
	{
		t++;
	}
	struct thread *tt = thread_current();
	printf("end %d %d %s\n",thread_current()->priority,++end_count,tt->name); //끝나는거알림
	int i,j,temp;
	for(i=0;i<tt->order_count;i++)
	{
		p2order[tt->order_array[i]] = tt->priority;
		porder[porder_count][0] = tt->order_array[i];
		porder[porder_count++][1] = tt->priority;
		j=0;		
		while(tt->name[j]!=NULL)
		{
			threadname[tt->order_array[i]][j] = tt->name[j++];
		}	
	}
	for(i=0;i<tt->time_count;i++)
	{
		time[time_count][0] = tt->time_array[i];
		time[time_count++][1] = tt->priority;
	}
}

