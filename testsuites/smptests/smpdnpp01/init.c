#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif
#include <sys/param.h>
#include <stdio.h>
#include <inttypes.h>
#include <rtems.h>
#include <rtems/libcsupport.h>
#include <rtems/score/schedulersmpimpl.h>
#include <rtems/score/smpbarrier.h>
#include <rtems/score/smplock.h>
#define TESTS_USE_PRINTK
#include "tmacros.h"
const char rtems_test_name[] = "SMPDNPP 1";
#define CPU_COUNT 2
#define SCHED_A rtems_build_name(' ', ' ', ' ', 'A')
#define SCHED_B rtems_build_name(' ', ' ', ' ', 'B')

static rtems_id scheduler_a;
static rtems_id scheduler_b;
// initially locked error test: the creating of the semaphore with the count 0 is forbidden for MPCP */ 
static void test_initially_locked_error(void)
{
  rtems_status_code sc;
  rtems_id sema_id;
  puts("test initially locked error");

  sc = rtems_semaphore_create(
       rtems_build_name('S', 'E', 'M', 'A'),
              0,
       RTEMS_BINARY_SEMAPHORE | RTEMS_PRIORITY |/** RTEMS_DISTRIBUTED_PRIORITY_CEILING,*/
   RTEMS_DISTRIBUTED_NO_PREEMPTIV,
       /**    RTEMS_MULTIPROCESSOR_PRIORITY_CEILING,
       /**      RTEMS_MULTIPROCESSOR_RESOURCE_SHARING
               | RTEMS_BINARY_SEMAPHORE,
               */ 1,
              &sema_id
       );
    if (sc == RTEMS_INVALID_NUMBER)
        puts("OK ");
    else
         printf("Can create initially locked semaphore: %s\n", rtems_status_text(sc));
}

/** rtems_semaphore_flush directive(unblocking all tasks waiting for the same resource) is forbidden for MPCP*/ 
static void test_flush_error(void)
{ 
  rtems_status_code sc;
  rtems_id id;
  rtems_task_priority prio;
  puts("test DPCP flush error");
  
  sc = rtems_semaphore_create(
       rtems_build_name('M', 'P', 'C', 'P'),
                    1,
   /**         RTEMS_MULTIPROCESSOR_RESOURCE_SHARING
                | RTEMS_BINARY_SEMAPHORE,*/
                       RTEMS_BINARY_SEMAPHORE | RTEMS_PRIORITY | RTEMS_DISTRIBUTED_NO_PREEMPTIV,

                       /**RTEMS_MULTIPROCESSOR_PRIORITY_CEILING,      
/** RTEMS_BINARY_SEMAPHORE | RTEMS_PRIORITY | RTEMS_DISTRIBUTED_PRIORITY_CEILING,*/               
                               1,
                                             &id
                                              );
 rtems_test_assert(sc == RTEMS_SUCCESSFUL);
 
 prio =1 ;
 sc = rtems_semaphore_set_priority(
          id,
          scheduler_b,
          prio,
          &prio
                );
   if (sc == RTEMS_SUCCESSFUL)
     puts("OK ");
   else
     printf("Can't set priority semaphore: %s\n", rtems_status_text(sc));    
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
  sc = rtems_semaphore_flush( id);
   if (sc == RTEMS_NOT_DEFINED)
      puts("OK ");
   else
    printf("Can't flush semaphore: %s\n", rtems_status_text(sc));

  sc = rtems_semaphore_delete( id);
  rtems_test_assert(sc == RTEMS_SUCCESSFUL);
}

static void Init(rtems_task_argument arg )
{
  rtems_status_code sc;
  rtems_resource_snapshot snapshot;
  TEST_BEGIN();

 rtems_resource_snapshot_take(&snapshot);

 sc = rtems_scheduler_ident(SCHED_A, &scheduler_a);
 rtems_test_assert(sc == RTEMS_SUCCESSFUL);
 
 sc = rtems_scheduler_ident(SCHED_B, &scheduler_b);
 rtems_test_assert(sc == RTEMS_SUCCESSFUL);
 
 test_flush_error();
 test_initially_locked_error();
 rtems_test_assert(rtems_resource_snapshot_check(&snapshot));
                  
 TEST_END();
 rtems_test_exit(0);
}

#define CONFIGURE_SMP_APPLICATION

#define CONFIGURE_MICROSECONDS_PER_TICK 1000

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_MAXIMUM_TASKS 2
/**#define CONFIGURE_MAXIMUM_SEMAPHORES 3*/
#define CONFIGURE_MAXIMUM_DNPP_SEMAPHORES 5 


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

*/

#define CONFIGURE_SCHEDULER_PRIORITY_MPCP_SMP

#include <rtems/scheduler.h>
 RTEMS_SCHEDULER_CONTEXT_MPCP_SMP(a, 256);
 RTEMS_SCHEDULER_CONTEXT_MPCP_SMP(b, 256);

#define CONFIGURE_SCHEDULER_CONTROLS \
 RTEMS_SCHEDULER_CONTROL_MPCP_SMP(a, SCHED_A), \
 RTEMS_SCHEDULER_CONTROL_MPCP_SMP(b, SCHED_B)

#define CONFIGURE_SMP_SCHEDULER_ASSIGNMENTS \
 RTEMS_SCHEDULER_ASSIGN(0, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_MANDATORY), \
 RTEMS_SCHEDULER_ASSIGN(1, RTEMS_SCHEDULER_ASSIGN_PROCESSOR_OPTIONAL)

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT

#include <rtems/confdefs.h>
