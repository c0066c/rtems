#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif
#include <stdio.h>
#include <inttypes.h>
#include <rtems.h>
#include <rtems/libcsupport.h>
#include <rtems/score/schedulersmpimpl.h>
#include <rtems/score/smplock.h>
#include <rtems/counter.h>
#include <rtems/cpuuse.h>
#include "tmacros.h"
#include "test_support.h"
#define TESTS_USE_PRINTK
#include <sys/param.h>
#include <rtems/score/smpbarrier.h>
#include <rtems/score/smplock.h>
const char rtems_test_name[] = "SMPMPCP 2";
#define SCHED_A rtems_build_name(' ', ' ', ' ', 'A')
#define SCHED_B rtems_build_name(' ', ' ', ' ', 'B')
 #define SCHED_C rtems_build_name(' ', ' ', ' ', 'C')
 #define SCHED_D rtems_build_name(' ', ' ', ' ', 'D')

#define SWITCH_EVENT_COUNT 32
#define MPCP_COUNT 32
#define CPU_COUNT 2 
typedef struct {
    uint32_t cpu_index;
    const Thread_Control *executing;
    const Thread_Control *heir;
    const Thread_Control *heir_node;
    Priority_Control heir_priority;
} switch_event;
typedef struct {
    rtems_id run_task_id;
    rtems_id mpcp_id;
    rtems_id high_task_id[32];
    rtems_id low_task_id[32];
    SMP_lock_Control switch_lock;
    rtems_id mpcp_ids[32];
    rtems_id scheduler_ids[CPU_COUNT];
    rtems_id main_task_id;
    size_t switch_index;   
    switch_event switch_events[32];
  } test_context;
static test_context test_instance = {
      .switch_lock = SMP_LOCK_INITIALIZER("test instance switch lock")
};
static void switch_extension(Thread_Control *executing, Thread_Control *heir)
{
     test_context *ctx = &test_instance;
     SMP_lock_Context lock_context;
     size_t i;
 
     _SMP_lock_ISR_disable_and_acquire(&ctx->switch_lock, &lock_context);
     i = ctx->switch_index;
    if (i < SWITCH_EVENT_COUNT) {
        switch_event *e = &ctx->switch_events[i];
         Scheduler_SMP_Node *node = _Scheduler_SMP_Thread_get_node(heir);
         e->cpu_index = rtems_get_current_processor();
         e->executing = executing;
         e->heir = heir;
         e->heir_node = _Scheduler_Node_get_owner(&node->Base);
         e->heir_priority = node->priority;
         ctx->switch_index = i + 1;
    }
    _SMP_lock_Release_and_ISR_enable(&ctx->switch_lock, &lock_context);
}

static void reset_switch_events(test_context *ctx)
{ 
    SMP_lock_Context lock_context;
    _SMP_lock_ISR_disable_and_acquire(&ctx->switch_lock, &lock_context);
    ctx->switch_index = 0;
    _SMP_lock_Release_and_ISR_enable(&ctx->switch_lock, &lock_context);
}

static size_t get_switch_events(test_context *ctx)
{
   SMP_lock_Context lock_context;
   size_t events;
 
   _SMP_lock_ISR_disable_and_acquire(&ctx->switch_lock, &lock_context);
   events = ctx->switch_index;
   _SMP_lock_Release_and_ISR_enable(&ctx->switch_lock, &lock_context);
   
   return events;
}

static void print_switch_events(test_context *ctx)
{
    size_t n = get_switch_events(ctx);
    size_t i;
    for (i = 0; i < n; ++i) {
     switch_event *e = &ctx->switch_events[i];
     char ex[5];
     char hr[5];
     char hn[5];
     rtems_object_get_name(e->executing->Object.id, sizeof(ex), &ex[0]);
     rtems_object_get_name(e->heir->Object.id, sizeof(hr), &hr[0]);
     rtems_object_get_name(e->heir_node->Object.id, sizeof(hn), &hn[0]);
   
     printf(
   "[%" PRIu32 "] %4s -> %4s (prio %3" PRIu64 ", node %4s)\n",
     e->cpu_index,
    &ex[0],
    &hr[0],
    e->heir_priority,
    &hn[0]
            );
      }
}

 static  void create_sema(
   test_context *ctx,
   rtems_id *id,
  rtems_task_priority prio
   )
 {
 uint32_t cpu_count = rtems_get_processor_count();
 uint32_t index;
 rtems_status_code sc;

 sc = rtems_semaphore_create(
   rtems_build_name('M', 'P', 'C', 'P'),
                    1,
  RTEMS_MULTIPROCESSOR_PRIORITY_CEILING
 /** RTEMS_PRIORITY_CEILING*/
 /** RTEMS_DISTRIBUTED_NO_PREEMPTIV*/
/**RTEMS_DISTRIBUTED_PRIORITY_CEILING*/
 |RTEMS_PRIORITY | RTEMS_BINARY_SEMAPHORE,
 /**RTEMS_BINARY_SEMAPHORE
             | RTEMS_MULTIPROCESSOR_RESOURCE_SHARING,*/
     prio,
     id
       );
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
       for (index = 1; index < cpu_count; index = ((index + 2) & ~UINT32_C(1))) {
               rtems_task_priority old_prio;

                   old_prio = 1;
                       sc = rtems_semaphore_set_priority(
                                     *id,
                                           ctx->scheduler_ids[index],
                                                 prio,
                                                       &old_prio
                                                           );
                           rtems_test_assert(sc == RTEMS_SUCCESSFUL);
                               rtems_test_assert(old_prio == 0);
                                 }
 }


static void obtain_and_release_task(rtems_task_argument arg)
{
   test_context *ctx = &test_instance;
   rtems_status_code sc;
   volatile bool *run_ = (volatile bool *) arg;
   *run_ = true;

   sc = rtems_semaphore_obtain(ctx->mpcp_ids[0], RTEMS_WAIT, RTEMS_NO_TIMEOUT);
   rtems_test_assert (sc==RTEMS_SUCCESSFUL);
   
   sc= rtems_task_wake_after(1);
   rtems_test_assert (sc == RTEMS_SUCCESSFUL);
   
   sc = rtems_semaphore_release(ctx->mpcp_ids[0]);
   rtems_test_assert(sc == RTEMS_SUCCESSFUL);
      while (true) {
                                                                                                                                                                           
       }
  } 

static void obtain_and_release_high(rtems_task_argument arg)
  {
     test_context *ctx = &test_instance;
     rtems_status_code sc;
     volatile bool *run_ = (volatile bool *) arg;
     *run_ = true;
     
     sc = rtems_semaphore_obtain(ctx->mpcp_ids[0], RTEMS_WAIT, RTEMS_NO_TIMEOUT);
     rtems_test_assert (sc==RTEMS_SUCCESSFUL);

     sc= rtems_task_wake_after(1);
     rtems_test_assert(sc==RTEMS_SUCCESSFUL);
   
    sc = rtems_semaphore_release(ctx->mpcp_ids[0]);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
      while (true) {
                                                                                                                                                                                                             }
       }

static void run_task(rtems_task_argument arg)
{
    volatile bool *run = (volatile bool *) arg;
    rtems_status_code sc;
    *run = true;

    sc=rtems_task_wake_after(2);
    rtems_test_assert (sc==RTEMS_SUCCESSFUL);
    while (true) {
                    }
    }

static void suspend_critical(rtems_task_argument arg)
{
   test_context *ctx = &test_instance;
   volatile bool *run_ = (volatile bool *) arg;
   *run_=true;
   rtems_status_code sc;

   sc = rtems_semaphore_obtain(ctx->mpcp_ids[6], RTEMS_WAIT, RTEMS_NO_TIMEOUT);
   if (sc == RTEMS_SUCCESSFUL)
    puts("OK ");
   else
    printf("Can't obtain semaphore: %s\n", rtems_status_text(sc));
   sc=rtems_task_wake_after(1);
   rtems_test_assert(sc == RTEMS_SUCCESSFUL);
 
   sc = rtems_semaphore_release(ctx->mpcp_ids[6]);
   rtems_test_assert (sc == RTEMS_SUCCESSFUL);
   while (true){
}
}

static void semaphore_first(rtems_task_argument arg)
{
    test_context *ctx = &test_instance;
    volatile bool *run_ = (volatile bool *) arg;
    *run_=true;
    rtems_status_code sc;
 
    sc= rtems_task_wake_after(3);
    rtems_test_assert (sc==RTEMS_SUCCESSFUL);
    
    sc = rtems_semaphore_obtain(ctx->mpcp_ids[10], RTEMS_WAIT, RTEMS_NO_TIMEOUT);
     if (sc == RTEMS_SUCCESSFUL)
        puts("OK ");
    else
        printf("Can't obtain semaphore: %s\n", rtems_status_text(sc));
   
    sc = rtems_task_wake_after(10);
    rtems_test_assert ( sc == RTEMS_SUCCESSFUL);
    
    sc=rtems_semaphore_release(ctx->mpcp_ids[10]);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    while (true) {
        }
     }
 
static void semaphore_second(rtems_task_argument arg)
 {
    test_context *ctx = &test_instance;
    volatile bool *run_ = (volatile bool *) arg;
    *run_=true;
    rtems_status_code sc;
   
    sc = rtems_semaphore_obtain(ctx->mpcp_ids[10], RTEMS_WAIT, RTEMS_NO_TIMEOUT);
     if (sc == RTEMS_SUCCESSFUL)
         puts("OK ");
     else
        printf("Can't obtain semaphore: %s\n", rtems_status_text(sc));
    sc=rtems_task_wake_after(5);
    rtems_test_assert(sc== RTEMS_SUCCESSFUL);
    
    sc=rtems_semaphore_release(ctx->mpcp_ids[10]);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    
    sc = rtems_task_wake_after(2);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    while (true) {
                        }
                    }

static void block_first(rtems_task_argument arg)
{
    test_context *ctx = &test_instance;
    volatile bool *run_ = (volatile bool *) arg;
    *run_=true;
    rtems_status_code sc;

    sc=rtems_task_wake_after(5);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    
    sc = rtems_semaphore_obtain(ctx->mpcp_ids[7], RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    sc = rtems_task_wake_after(5);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    
    sc=rtems_semaphore_release(ctx->mpcp_ids[7]);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    while (true) {
 }
}

static void block_second(rtems_task_argument arg)
{
    test_context *ctx = &test_instance;
    volatile bool *run_ = (volatile bool *) arg;
    *run_=true;
    rtems_status_code sc;
                          
    sc = rtems_semaphore_obtain(ctx->mpcp_ids[8], RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    rtems_task_wake_after(1);
   
    sc=rtems_semaphore_release(ctx->mpcp_ids[8]);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    while (true) {
         }
       }

static void obtain_release_global(rtems_task_argument arg)
{
    test_context *ctx = &test_instance;
    volatile bool *run_ = (volatile bool *) arg;
    *run_=true;
    rtems_status_code sc;
    
    sc = rtems_semaphore_obtain(ctx->mpcp_ids[4], RTEMS_WAIT, RTEMS_NO_TIMEOUT);
     if (sc == RTEMS_SUCCESSFUL)
       puts("OK ");
     else
       printf("Can't obtain semaphore: %s\n", rtems_status_text(sc));
 
    rtems_task_wake_after(1);
    sc=rtems_semaphore_release(ctx->mpcp_ids[4]);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    while (true) {
           }
         }

/** test function proves the assigment the ceiling prioority to the task by obtaining a resource.
 * After rtems_semaphore_release directive,the tasks rerurns its nominal priority*/
static void test_mpcp_ceiling(test_context *ctx)
{
  uint32_t    start;
  rtems_status_code sc;
  rtems_task_priority prio;
  rtems_id scheduler_id;
 
  rtems_cpu_usage_reset();
  reset_switch_events(ctx);
  
  volatile bool run=false;
  uint32_t cpu_count = rtems_get_processor_count();
 
  puts("test MPCP ceiling priority ");
  sc = rtems_task_create(
  rtems_build_name('H', 'I', 'G', 'H'),
                         2,
   RTEMS_MINIMUM_STACK_SIZE,
   RTEMS_DEFAULT_MODES,
   RTEMS_DEFAULT_ATTRIBUTES,
   &ctx->high_task_id[0]
              );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

 sc = rtems_task_get_scheduler(RTEMS_SELF, &scheduler_id);
 rtems_test_assert(sc == RTEMS_SUCCESSFUL);

 rtems_test_assert(ctx->scheduler_ids[0] == scheduler_id);
 sc = rtems_task_create(
           rtems_build_name(' ', 'L', 'O', 'W'),
         4,
 RTEMS_MINIMUM_STACK_SIZE,
 RTEMS_DEFAULT_MODES,
 RTEMS_DEFAULT_ATTRIBUTES,
 &ctx->low_task_id[0]
    );
 rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  
 sc = rtems_task_get_scheduler(ctx->low_task_id[0], &scheduler_id);
 rtems_test_assert(sc == RTEMS_SUCCESSFUL);

 rtems_test_assert(ctx->scheduler_ids[0] == scheduler_id);
 
 create_sema(ctx, &ctx->mpcp_ids[0], 1);
                      
    if (cpu_count > 1) {
        sc = rtems_task_set_scheduler(ctx->high_task_id[0], ctx->scheduler_ids[1],2);
    if (sc == RTEMS_SUCCESSFUL)
        puts("OK ");
    else
        printf("Can't set scheduler: %s\n", rtems_status_text(sc));
}
  sc = rtems_task_start (ctx->low_task_id[0], obtain_and_release_task,(rtems_task_argument) &run);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  rtems_test_assert(!run);
  sc = rtems_task_wake_after(5);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  
  sc = rtems_task_start(ctx->high_task_id[0], obtain_and_release_high,(rtems_task_argument) &run );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  rtems_test_assert(run);
  run=false;
  rtems_task_wake_after(5);
 
  print_switch_events(ctx);

  rtems_cpu_usage_report();

  sc = rtems_task_delete(ctx->low_task_id[0]);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
 
  sc = rtems_task_delete(ctx->high_task_id[0]);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
 
  sc = rtems_semaphore_delete(ctx->mpcp_ids[0]);
     if (sc == RTEMS_SUCCESSFUL)
        puts("OK ");
    else
        printf("Can't delete semaphore: %s\n", rtems_status_text(sc));
}
/** test function proves the blocking of the lower nominal priority task by the task 
 * with the higher nominal priority, when the tasks access same resource on the different cpu*/
 static void test_mpcp_critical_semaphore(test_context *ctx)
{
    rtems_status_code sc;
    reset_switch_events(ctx);
    rtems_cpu_usage_reset();
    volatile bool run = false;
    
    rtems_task_priority prio;
    
    puts("test MPCP critical semaphore");
    sc = rtems_task_create(
    rtems_build_name(' ', 'L', 'O', 'W'),
                               5,
     RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
     &ctx->low_task_id[4]
           );
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    
    sc = rtems_task_create(
    rtems_build_name('H', 'I', 'G', 'H'),
                       2,
     RTEMS_MINIMUM_STACK_SIZE,
     RTEMS_DEFAULT_MODES,
     RTEMS_DEFAULT_ATTRIBUTES,
     &ctx->high_task_id[4]
                    );
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    
    sc = rtems_task_set_scheduler(ctx->high_task_id[4],ctx->scheduler_ids[1],4);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    create_sema(ctx, &ctx->mpcp_ids[10],1 );
 
    sc = rtems_task_start(ctx->low_task_id[4],semaphore_first,(rtems_task_argument) &run);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
 
     sc= rtems_task_start(ctx->high_task_id[4],semaphore_second,(rtems_task_argument) &run);
     rtems_test_assert(sc == RTEMS_SUCCESSFUL);
     
     rtems_task_wake_after(5);
     rtems_test_assert(run);
     run=false;
     rtems_test_assert(!run);
    
    rtems_cpu_usage_report();
    print_switch_events(ctx);
    
    sc = rtems_semaphore_delete(ctx->mpcp_ids[10]);
        if (sc == RTEMS_SUCCESSFUL)
            puts("OK ");
        else
             printf("Can't delete semaphore: %s\n", rtems_status_text(sc));
 
sc= rtems_task_delete(ctx->high_task_id[4]);
rtems_test_assert(sc == RTEMS_SUCCESSFUL);

sc = rtems_task_delete(ctx->low_task_id[4]);
rtems_test_assert (sc== RTEMS_SUCCESSFUL);
             }
/** this function proves the preemption of the critical section by another critical section with higher ceiling */ 
static void test_mpcp_block_critical(test_context *ctx)
     {
    rtems_status_code sc;
    reset_switch_events(ctx);
    rtems_cpu_usage_reset();
    volatile bool run = false;
    rtems_task_priority prio;
 
    puts("test MPCP block critical");
  puts ("");
  puts ("");

    sc = rtems_task_create(
    rtems_build_name(' ', 'L', 'O', 'W'),
                      5,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
    &ctx->low_task_id[5]
         );
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
   
    sc = rtems_task_create(
    rtems_build_name('H', 'I', 'G', 'H'),
                  4,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
    &ctx->high_task_id[5]
                );
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    
    sc = rtems_task_set_scheduler(ctx->low_task_id[5],ctx->scheduler_ids[1],5);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    
    sc = rtems_task_set_scheduler(ctx->high_task_id[5],ctx->scheduler_ids[1],4);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    
    create_sema(ctx, &ctx->mpcp_ids[7],2 );
    create_sema (ctx,&ctx->mpcp_ids[8],1);
    
    sc = rtems_task_start(ctx->high_task_id[5],block_first,(rtems_task_argument) &run);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    
    sc= rtems_task_start(ctx->low_task_id[5],block_second,(rtems_task_argument) &run);
    /** if (sc== RTEMS_SUCCESSFUL)
         puts ("OK");
    else
         printf ("Can't start task: %s\n", rtems_status_text(sc));*/
    rtems_test_assert(run);
    sc = rtems_task_wake_after(20);
    /** if (sc == RTEMS_SUCCESSFUL)
         puts("OK ");
    else
         printf("Can't sleep task: %s\n", rtems_status_text(sc));*/
    rtems_test_assert(run);
    run=false;
    rtems_task_wake_after(5);
    
    print_switch_events(ctx);
    rtems_cpu_usage_report();

    sc = rtems_semaphore_delete(ctx->mpcp_ids[7]);
    /** if (sc == RTEMS_SUCCESSFUL)
       puts("OK ");
     else
       printf("Can't delete semaphore: %s\n", rtems_status_text(sc));*/
     sc= rtems_semaphore_delete(ctx->mpcp_ids[8]);
     rtems_test_assert(sc == RTEMS_SUCCESSFUL);
     
     sc= rtems_task_delete(ctx->high_task_id[5]);
     rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  
     sc = rtems_task_delete(ctx->low_task_id[5]);
     rtems_test_assert (sc== RTEMS_SUCCESSFUL);
    }
/* function tests the preemprion the task in normal execution by the task in the critical section on the same cpu */
static void test_mpcp_obtain_critical(test_context *ctx)
{
    rtems_status_code sc;
    reset_switch_events(ctx);
    rtems_cpu_usage_reset();
    volatile bool run = false;
    rtems_task_priority prio;
     puts("test MPCP obtain critical");
    sc = rtems_task_create(
         rtems_build_name(' ', 'L', 'O', 'W'),
                            5,
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
        &ctx->low_task_id[6]
                    );
   rtems_test_assert(sc == RTEMS_SUCCESSFUL);
   
   sc = rtems_task_create(
   rtems_build_name('H', 'I', 'G', 'H'),
                          4,
        RTEMS_MINIMUM_STACK_SIZE,
        RTEMS_DEFAULT_MODES,
        RTEMS_DEFAULT_ATTRIBUTES,
       &ctx->high_task_id[6]
                     );
   rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  
   sc = rtems_task_set_scheduler(ctx->low_task_id[6],ctx->scheduler_ids[1],5);
   rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  
   sc = rtems_task_set_scheduler(ctx->high_task_id[6],ctx->scheduler_ids[1],4);
   rtems_test_assert(sc == RTEMS_SUCCESSFUL);
   
   create_sema(ctx, &ctx->mpcp_ids[6],1 );
 
   sc = rtems_task_start(ctx->high_task_id[6],run_task,(rtems_task_argument) &run);
   rtems_test_assert(sc == RTEMS_SUCCESSFUL);

   sc= rtems_task_start(ctx->low_task_id[6],suspend_critical,(rtems_task_argument) &run);
   rtems_test_assert(sc == RTEMS_SUCCESSFUL);
   rtems_test_assert(run);
   sc = rtems_task_wake_after(10);
         if (sc == RTEMS_SUCCESSFUL)
           puts("OK ");
                else
          printf("Can't sleep task: %s\n", rtems_status_text(sc));

    sc = rtems_semaphore_obtain(ctx->mpcp_ids[6], RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    rtems_task_wake_after(5);
    
    sc = rtems_semaphore_release(ctx->mpcp_ids[6]);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    
       print_switch_events(ctx);
     rtems_cpu_usage_report();

    sc = rtems_semaphore_delete(ctx->mpcp_ids[6]);
    if (sc == RTEMS_SUCCESSFUL)
      puts("OK ");
    else
     printf("Can't delete semaphore: %s\n", rtems_status_text(sc));
    
    sc = rtems_task_delete(ctx->high_task_id[6]);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
 
    sc = rtems_task_delete(ctx->low_task_id[6]);
    rtems_test_assert (sc== RTEMS_SUCCESSFUL);
                          }
/**test function to measure overhead */
static void test_overhead(test_context *ctx)
{
    volatile bool run = false;
    rtems_status_code sc;
    uint32_t    start, end,gap,max,counter;
    max = 0;
    gap=1;
    counter=0; 
    reset_switch_events(ctx);
 
    rtems_cpu_usage_reset();
  
    sc=rtems_task_create(
           rtems_build_name(' ', 'R', 'U', 'N'),
           6,
           RTEMS_MINIMUM_STACK_SIZE,
           RTEMS_DEFAULT_MODES,
           RTEMS_DEFAULT_ATTRIBUTES,
           &ctx->high_task_id[12]
     );
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    
   /** sc = rtems_task_set_scheduler(ctx->high_task_id[12], ctx->scheduler_ids[1],6);
    rtems_test_assert( sc == RTEMS_SUCCESSFUL);
   */
    sc = rtems_task_start(ctx->high_task_id[12],run_task, (rtems_task_argument) &run);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    
    create_sema(ctx, &ctx->mpcp_ids[12],2 );
  
   /** while (counter!=1000) {
     if(gap>max){
        max=gap;
        printf("Overhead by obtain: %d\n", gap);
  } 
  start= rtems_clock_get_ticks_since_boot();
   */
  sc = rtems_semaphore_obtain(ctx->mpcp_ids[12], RTEMS_NO_WAIT, RTEMS_NO_TIMEOUT);
     if (sc == RTEMS_SUCCESSFUL)
         puts("OK ");
     else
   printf("Can't obtain semaphore: %s\n", rtems_status_text(sc));
 /** end = rtems_clock_get_ticks_since_boot();
            gap=(end-start);
 counter++;}
    */     
    rtems_test_assert(!run);
    sc = rtems_task_wake_after(1);
    if (sc == RTEMS_SUCCESSFUL)
        puts("OK ");
    else
        printf("Can't sleep task: %s\n", rtems_status_text(sc));
    
      rtems_test_assert(!run);

  
      while (counter!=1000) {
        if(gap>max){
          max=gap;
         printf("Overhead by release: %d\n", gap);
       }  start= rtems_clock_get_ticks_since_boot();
          sc = rtems_semaphore_release(ctx->mpcp_ids[12]);
  if (sc == RTEMS_SUCCESSFUL)
     puts("OK ");
 else
    printf("Can't release semaphore: %s\n", rtems_status_text(sc));
end = rtems_clock_get_ticks_since_boot();
gap = (end-start);
 counter++;
}
 rtems_test_assert(!run);
 sc = rtems_task_wake_after(5);

 
 rtems_cpu_usage_report();

 print_switch_events(ctx);
 
 sc = rtems_semaphore_delete(ctx->mpcp_ids[12]);
 rtems_test_assert(sc == RTEMS_SUCCESSFUL);

 sc = rtems_task_delete(ctx->high_task_id[12]);
 rtems_test_assert(sc == RTEMS_SUCCESSFUL);

}

/** function tests multiple obtain by MPCP */
static void test_mpcp_multiple_access(test_context *ctx)
{
  rtems_status_code sc;
  rtems_id run_task_id;
  reset_switch_events(ctx);
  rtems_cpu_usage_reset();
  rtems_id sem_id;
  volatile bool run = false;
  rtems_task_priority prio; 
  puts("test MPCP multiple access");
 
  sc = rtems_task_create(
  rtems_build_name(' ', 'R', 'U', 'N'),
          6,
    RTEMS_MINIMUM_STACK_SIZE,
    RTEMS_DEFAULT_MODES,
    RTEMS_DEFAULT_ATTRIBUTES,
    &ctx->run_task_id
                 );
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  sc = rtems_task_set_scheduler(ctx->run_task_id, ctx->scheduler_ids[1],6);
 
  create_sema(ctx, &ctx->mpcp_ids[4],2 );
  create_sema(ctx, &ctx->mpcp_ids[5],3 );
 
  sc = rtems_task_start(ctx->run_task_id,obtain_release_global, (rtems_task_argument) &run);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  
  rtems_test_assert(run);
  sc = rtems_task_wake_after(3);
  if (sc == RTEMS_SUCCESSFUL)
        puts("OK ");
        else
                   printf("Can't sleep task: %s\n", rtems_status_text(sc));
   rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  rtems_test_assert(run);
  run = false;
  sc = rtems_semaphore_obtain(ctx->mpcp_ids[4], RTEMS_WAIT, RTEMS_NO_TIMEOUT);
  rtems_test_assert (sc == RTEMS_SUCCESSFUL);
  if (sc == RTEMS_SUCCESSFUL)
      puts("OK ");
          else
      printf("Can't obtain semaphore: %s\n", rtems_status_text(sc));
   rtems_test_assert(!run);
   sc = rtems_task_wake_after(1);
   rtems_test_assert(sc == RTEMS_SUCCESSFUL);
   rtems_test_assert(!run);
   
   sc = rtems_semaphore_release(ctx->mpcp_ids[4]);
   rtems_test_assert(sc == RTEMS_SUCCESSFUL);
   rtems_test_assert(!run);
   
   sc = rtems_task_wake_after(2);
     if (sc == RTEMS_SUCCESSFUL)
        puts("OK ");
    else
        printf("Can't sleep task: %s\n", rtems_status_text(sc));
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  rtems_test_assert(!run);
  run = false;
  sc = rtems_semaphore_obtain(ctx->mpcp_ids[5], RTEMS_WAIT, RTEMS_NO_TIMEOUT);
  rtems_test_assert (sc == RTEMS_SUCCESSFUL);
   
  rtems_test_assert(!run);
  sc = rtems_task_wake_after(1);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  
  rtems_test_assert(!run);
  sc = rtems_semaphore_release(ctx->mpcp_ids[5]);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
 
  rtems_test_assert(!run);
  rtems_task_wake_after(2);
 
  rtems_cpu_usage_report();
  print_switch_events(ctx);
  sc = rtems_semaphore_delete(ctx->mpcp_ids[4]);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  sc = rtems_semaphore_delete(ctx->mpcp_ids[5]);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
 
  sc = rtems_task_delete(ctx->run_task_id);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
}

static void test_nested_obtain_error(test_context *ctx)
{
  rtems_status_code sc;
  rtems_id id;

  puts("test MPCP nested obtain error");
  create_sema(ctx, &id, 1);

  sc = rtems_semaphore_obtain(id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  sc = rtems_semaphore_obtain(id, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
  rtems_test_assert(sc == RTEMS_UNSATISFIED);

  sc = rtems_semaphore_release(id);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

  sc = rtems_semaphore_delete(id);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
}


static void Init(rtems_task_argument arg)
{
  test_context *ctx = &test_instance;
  rtems_status_code sc;
  uint32_t cpu_index;
  rtems_resource_snapshot snapshot;
   TEST_BEGIN();
  uint32_t cpu_count = rtems_get_processor_count();
  printf ("CPU count in your system: %d\n", cpu_count);
  ctx->main_task_id = rtems_task_self();
  uint32_t tick_per_second = rtems_clock_get_ticks_per_second();
   printf( "\nTicks per second in your system: %" PRIu32 "\n", tick_per_second );
  rtems_resource_snapshot_take(&snapshot);
 /** for (cpu_index = 0; cpu_index < cpu_count; ++cpu_index) {
            sc = rtems_scheduler_ident(cpu_index, &ctx->scheduler_ids[cpu_index]);
                rtems_test_assert(sc == RTEMS_SUCCESSFUL);
                  }
*/
 sc = rtems_scheduler_ident(SCHED_A,&ctx->scheduler_ids[0]);
 rtems_test_assert(sc == RTEMS_SUCCESSFUL);
 
 sc = rtems_scheduler_ident(SCHED_B,&ctx->scheduler_ids[1]);
 rtems_test_assert(sc == RTEMS_SUCCESSFUL);
/**
 sc = rtems_scheduler_ident(SCHED_C,&ctx->scheduler_ids[2]);
 rtems_test_assert(sc == RTEMS_SUCCESSFUL);

 sc = rtems_scheduler_ident(SCHED_D,&ctx->scheduler_ids[3]);
 rtems_test_assert(sc == RTEMS_SUCCESSFUL);
*/


/** test_mpcp_ceiling(ctx);*/
/**test_mpcp_multiple_access(ctx);*/
 /** test_mpcp_obtain_critical(ctx);*/
test_mpcp_block_critical(ctx);
/**test_mpcp_critical_semaphore(ctx);*/
/**test_overhead(ctx);
/**test_nested_obtain_error(ctx);
*/
 rtems_test_assert(rtems_resource_snapshot_check(&snapshot));
  TEST_END();
 rtems_test_exit(0);
}
#define CONFIGURE_SMP_APPLICATION
#define CONFIGURE_MICROSECONDS_PER_TICK 1000

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER


#define CONFIGURE_MAXIMUM_TASKS (2 * CPU_COUNT + 2)
#define CONFIGURE_MAXIMUM_SEMAPHORES (MPCP_COUNT + 1)
#define CONFIGURE_MAXIMUM_MPCP_SEMAPHORES MPCP_COUNT
#define CONFIGURE_SMP_MAXIMUM_PROCESSORS CPU_COUNT

 #define CONFIGURE_MAXIMUM_PRIORITY 255

/**
#define CONFIGURE_SCHEDULER_PRIORITY_SMP

#include <rtems/scheduler.h>
 RTEMS_SCHEDULER_CONTEXT_PRIORITY_SMP(a, CONFIGURE_MAXIMUM_PRIORITY + 1);
 RTEMS_SCHEDULER_CONTEXT_PRIORITY_SMP(b, CONFIGURE_MAXIMUM_PRIORITY + 1);



#define CONFIGURE_SCHEDULER_CONTROLS \
 RTEMS_SCHEDULER_CONTROL_PRIORITY_SMP(a, SCHED_A), \
 RTEMS_SCHEDULER_CONTROL_PRIORITY_SMP(b, SCHED_B)
 

  #define CONFIGURE_SMP_SCHEDULER_ASSIGNMENTS \ 
  RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
      RTEMS_SCHEDULER_ASSIGN(1, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL)

*/
#define CONFIGURE_SCHEDULER_PRIORITY_MPCP_SMP
 #include <rtems/scheduler.h>
 RTEMS_SCHEDULER_CONTEXT_MPCP_SMP(a,  CONFIGURE_MAXIMUM_PRIORITY + 1);
 RTEMS_SCHEDULER_CONTEXT_MPCP_SMP(b,  CONFIGURE_MAXIMUM_PRIORITY + 1);
 #define CONFIGURE_SCHEDULER_CONTROLS \
     RTEMS_SCHEDULER_CONTROL_MPCP_SMP(a, SCHED_A),\
     RTEMS_SCHEDULER_CONTROL_MPCP_SMP(b, SCHED_B)
 #define CONFIGURE_SMP_SCHEDULER_ASSIGNMENTS \
    RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY),\
    RTEMS_SCHEDULER_ASSIGN(1, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL)


/**
#define CONFIGURE_SCHEDULER_SIMPLE_SMP

#include <rtems/scheduler.h>

RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(0);
RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(1);
RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(2);
RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(3);
RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(4);
RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(5);
RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(6);
RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(7);
RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(8);
RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(9);
RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(10);
RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(11);
RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(12);
RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(13);
RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(14);
RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(15);
RTEMS_SCHEDULER_CONTEXT_SIMPLE_SMP(16);

#define CONFIGURE_SCHEDULER_CONTROLS \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(0, 0), \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(1, 1), \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(2, 2), \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(3, 3), \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(4, 4), \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(5, 5), \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(6, 6), \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(7, 7), \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(8, 8), \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(9, 9), \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(10, 10), \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(11, 11), \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(12, 12), \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(13, 13), \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(14, 14), \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(15, 15), \
  RTEMS_SCHEDULER_CONTROL_SIMPLE_SMP(16, 16)

#define CONFIGURE_SMP_SCHEDULER_ASSIGNMENTS \
  RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
  RTEMS_SCHEDULER_ASSIGN(1, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(2, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(2, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(3, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(3, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(4, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(4, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(5, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(5, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(6, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(6, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(7, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(7, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(8, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(8, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(9, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(9, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(10, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(10, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(11, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(11, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(12, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(12, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(13, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(13, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(14, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(14, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(15, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(15, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(16, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL), \
  RTEMS_SCHEDULER_ASSIGN(16, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL)
*/
#define CONFIGURE_INITIAL_EXTENSIONS \
       { .thread_switch = switch_extension }, \
   RTEMS_TEST_INITIAL_EXTENSION

#define CONFIGURE_INIT_TASK_NAME rtems_build_name('M', 'A', 'I', 'N')
#define CONFIGURE_INIT_TASK_PRIORITY 3

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE

#define CONFIGURE_INIT

#include <rtems/confdefs.h>
