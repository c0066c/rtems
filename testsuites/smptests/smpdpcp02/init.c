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
const char rtems_test_name[] = "SMPDPCP 02";
#define SCHED_A rtems_build_name(' ', ' ', ' ', 'A')
#define SCHED_B rtems_build_name(' ', ' ', ' ', 'B')
#define SWITCH_EVENT_COUNT 32
#define DPCP_COUNT 32
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
    rtems_id dpcp_ids[32];
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
/**rtems_task_wake_after(1);*/
   sc = rtems_semaphore_obtain(ctx->dpcp_ids[6], RTEMS_WAIT, RTEMS_NO_TIMEOUT);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);

   sc=rtems_task_wake_after(1);
   rtems_test_assert(sc == RTEMS_SUCCESSFUL);
   
   sc = rtems_semaphore_release(ctx->dpcp_ids[6]);
   rtems_test_assert(sc == RTEMS_SUCCESSFUL);
      
   while (true){
}
}

            

static void block_first(rtems_task_argument arg)
{
    test_context *ctx = &test_instance;
    volatile bool *run_ = (volatile bool *) arg;
    *run_=true;
    rtems_status_code sc;

    sc=rtems_task_wake_after(2);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    sc = rtems_semaphore_obtain(ctx->dpcp_ids[7], RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    
    sc = rtems_task_wake_after(10);
        rtems_test_assert(sc == RTEMS_SUCCESSFUL);
   
    sc=rtems_semaphore_release(ctx->dpcp_ids[7]);
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
    rtems_task_wake_after(1);              
    sc = rtems_semaphore_obtain(ctx->dpcp_ids[8], RTEMS_WAIT, RTEMS_NO_TIMEOUT);
          rtems_task_wake_after(1);

    sc=rtems_semaphore_release(ctx->dpcp_ids[8]);
/**   rtems_task_wake_after(1);*/
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    while (true) {
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
     rtems_build_name('D', 'P', 'C', 'P'),
                1,
/**RTEMS_MULTIPROCESSOR_PRIORITY_CEILING
/** RTEMS_PRIORITY_CEILING*/
              RTEMS_DISTRIBUTED_PRIORITY_CEILING
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
/**    rtems_test_assert(old_prio == 0);*/
        }
}


static void test_critical(test_context *ctx)
     {
    rtems_status_code sc;
    reset_switch_events(ctx);
    volatile bool run = false;
    rtems_task_priority prio;
 
    puts("test DPCP critical");
   
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
  
    create_sema(ctx, &ctx->dpcp_ids[7],2);
    create_sema (ctx,&ctx->dpcp_ids[8],1);
    
    sc = rtems_task_start(ctx->high_task_id[5],block_first,(rtems_task_argument) &run);
        
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    
    sc= rtems_task_start(ctx->low_task_id[5],block_second,(rtems_task_argument) &run);
    rtems_test_assert(!run);
    sc = rtems_task_wake_after(10);
   rtems_test_assert(run);
  run=false;
  sc=rtems_semaphore_obtain(ctx->dpcp_ids[7], RTEMS_WAIT, RTEMS_NO_TIMEOUT);

  rtems_task_wake_after(10);

  sc = rtems_semaphore_release(ctx->dpcp_ids[7]);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
       /**       
    rtems_test_assert(!run);               
rtems_task_wake_after(1);
*/
print_switch_events(ctx);
    
    sc = rtems_semaphore_delete(ctx->dpcp_ids[7]);
     sc= rtems_semaphore_delete(ctx->dpcp_ids[8]);
     
     sc= rtems_task_delete(ctx->high_task_id[5]);
  
     sc = rtems_task_delete(ctx->low_task_id[5]);

    }

 static void test_normal(test_context *ctx)
 {
   rtems_status_code sc;
   reset_switch_events(ctx);
    volatile bool run = false;
   rtems_task_priority prio;
   puts("test DPCP normal");
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

   create_sema(ctx, &ctx->dpcp_ids[6],2 );
   sc= rtems_task_start(ctx->high_task_id[6],run_task,(rtems_task_argument) &run);
      rtems_test_assert(sc == RTEMS_SUCCESSFUL);
    sc = rtems_task_start(ctx->low_task_id[6],suspend_critical,(rtems_task_argument) &run);
         rtems_test_assert(sc == RTEMS_SUCCESSFUL);

        rtems_test_assert(!run);
     sc = rtems_task_wake_after(2);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);
       
          sc = rtems_semaphore_obtain(ctx->dpcp_ids[6], RTEMS_WAIT, RTEMS_NO_TIMEOUT);
     rtems_test_assert(sc == RTEMS_SUCCESSFUL);
     rtems_task_wake_after(5);
    sc = rtems_semaphore_release(ctx->dpcp_ids[6]);
    rtems_test_assert(sc == RTEMS_SUCCESSFUL);

    rtems_test_assert(run);
    rtems_task_wake_after(10);
   
     print_switch_events(ctx);
    sc = rtems_semaphore_delete(ctx->dpcp_ids[6]);
     rtems_test_assert(sc == RTEMS_SUCCESSFUL);

   sc = rtems_task_delete(ctx->high_task_id[6]);
     rtems_test_assert(sc == RTEMS_SUCCESSFUL);

   sc = rtems_task_delete(ctx->low_task_id[6]);
     rtems_test_assert(sc == RTEMS_SUCCESSFUL);
 }

static void test_dpcp_nested_obtain_error(test_context *ctx)
{
  rtems_status_code sc;
  rtems_id id;

  puts("test DPCP nested obtain error");

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
 sc = rtems_scheduler_ident(SCHED_A,&ctx->scheduler_ids[0]);
 rtems_test_assert(sc == RTEMS_SUCCESSFUL);
 
 sc = rtems_scheduler_ident(SCHED_B,&ctx->scheduler_ids[1]);
 rtems_test_assert(sc == RTEMS_SUCCESSFUL);
/**test_normal(ctx);*/
test_critical(ctx);

/**test_dpcp_nested_obtain_error(ctx);*/
rtems_test_assert(rtems_resource_snapshot_check(&snapshot));
  TEST_END();
 rtems_test_exit(0);
}
#define CONFIGURE_SMP_APPLICATION
#define CONFIGURE_MICROSECONDS_PER_TICK 1000

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER


#define CONFIGURE_MAXIMUM_TASKS (2 * CPU_COUNT + 2)
#define CONFIGURE_MAXIMUM_SEMAPHORES (DPCP_COUNT + 1)
#define CONFIGURE_MAXIMUM_MPCP_SEMAPHORES DPCP_COUNT
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
/**
#define CONFIGURE_SCHEDULER_PRIORITY_MPCP_SMP
 #include <rtems/scheduler.h>
RTEMS_SCHEDULER_CONTEXT_MPCP_SMP(a, CONFIGURE_MAXIMUM_PRIORITY+1);
RTEMS_SCHEDULER_CONTEXT_MPCP_SMP(b, CONFIGURE_MAXIMUM_PRIORITY+1);
 #define CONFIGURE_SCHEDULER_CONTROLS \
     RTEMS_SCHEDULER_CONTROL_MPCP_SMP(a, SCHED_A),\
     RTEMS_SCHEDULER_CONTROL_MPCP_SMP(b, SCHED_B)
 #define CONFIGURE_SMP_SCHEDULER_ASSIGNMENTS \
    RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY),\
    RTEMS_SCHEDULER_ASSIGN(1, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL) 
 */
#define CONFIGURE_SCHEDULER_PRIORITY_DPCP_SMP
  #include <rtems/scheduler.h>
 RTEMS_SCHEDULER_CONTEXT_DPCP_SMP(a, CONFIGURE_MAXIMUM_PRIORITY+1);
  RTEMS_SCHEDULER_CONTEXT_DPCP_SMP(b, CONFIGURE_MAXIMUM_PRIORITY+1);
    #define CONFIGURE_SCHEDULER_CONTROLS \
            RTEMS_SCHEDULER_CONTROL_DPCP_SMP(a, SCHED_A),\
        RTEMS_SCHEDULER_CONTROL_DPCP_SMP(b, SCHED_B)
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
