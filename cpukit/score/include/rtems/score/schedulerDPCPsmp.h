#ifndef _RTEMS_SCORE_SCHEDULER_DPCP_SMP_H
#define _RTEMS_SCORE_SCHEDULER_DPCP_SMP_H
#include <rtems/score/scheduler.h>
#include <rtems/score/schedulerpriority.h>
#include <rtems/score/schedulersmp.h>
#include <rtems/score/schedulerprioritysmp.h>
#include <rtems/score/cpuset.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**entry points for the DPCP scheduler */
#define SCHEDULER_DPCP_SMP_ENTRY_POINTS \
{ \
_Scheduler_priority_SMP_Initialize, \
     _Scheduler_default_Schedule, \
     _Scheduler_priority_SMP_Yield, \
    _Scheduler_DPCP_SMP_Block, \
    _Scheduler_DPCP_SMP_Unblock, \
     _Scheduler_DPCP_SMP_Update_priority, \
     _Scheduler_default_Map_priority, \
     _Scheduler_default_Unmap_priority, \
     _Scheduler_default_Ask_for_help, \
     _Scheduler_default_Reconsider_help_request, \
     _Scheduler_DPCP_SMP_Withdraw_node, \
     _Scheduler_DPCP_SMP_Add_processor, \
     _Scheduler_DPCP_SMP_Remove_processor, \
     _Scheduler_DPCP_SMP_Node_initialize, \
    _Scheduler_default_Node_destroy, \
    _Scheduler_default_Release_job, \
     _Scheduler_default_Cancel_job, \
     _Scheduler_default_Tick, \
    _Scheduler_SMP_Start_idle \
     SCHEDULER_OPERATION_DEFAULT_GET_SET_AFFINITY \
                                                   }
 void _Scheduler_DPCP_SMP_Node_initialize(
 const Scheduler_Control *scheduler,
       Scheduler_Node          *node,
       Thread_Control          *the_thread,
       Priority_Control         priority
               );
 
 void _Scheduler_DPCP_SMP_Block(
  const Scheduler_Control *scheduler,
        Thread_Control          *thread,
        Scheduler_Node          *node
           );

 void _Scheduler_DPCP_SMP_Unblock(
   const Scheduler_Control *scheduler,
        Thread_Control          *thread,
        Scheduler_Node          *node
              );

 void _Scheduler_DPCP_SMP_Update_priority(
   const Scheduler_Control *scheduler,
         Thread_Control          *the_thread,
         Scheduler_Node          *node
              );

 void _Scheduler_DPCP_SMP_Withdraw_node(
  const Scheduler_Control *scheduler,
       Thread_Control          *the_thread,
       Scheduler_Node          *node,
       Thread_Scheduler_state   next_state
              );

void _Scheduler_DPCP_SMP_Add_processor(
    const Scheduler_Control *scheduler,
          Thread_Control          *idle
              );

Thread_Control *_Scheduler_DPCP_SMP_Remove_processor(
    const Scheduler_Control *scheduler,
    struct Per_CPU_Control  *cpu
              );

typedef struct {
   Scheduler_SMP_Node Base; 
   Scheduler_priority_Ready_queue Ready_queue;
} Scheduler_DPCP_SMP_Node;

 



 /** @} */

#ifdef __cplusplus
      }
#endif /* __cplusplus */

#endif
