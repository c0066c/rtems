#ifndef _RTEMS_SCORE_DPCPIMPL_H
#define _RTEMS_SCORE_DPCPIMPL_H

#include <rtems/score/dpcp.h>

#if defined(RTEMS_SMP)

#include <rtems/score/assert.h>
#include <rtems/score/status.h>
#include <rtems/score/threadqimpl.h>
#include <rtems/score/watchdogimpl.h>
#include <rtems/score/wkspace.h>
#include <rtems/score/chainimpl.h>
#include <rtems/score/schedulerimpl.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     *  * @addtogroup ScoreDPCP
     *   *
     *    * @{
     *     */
#define DPCP_TQ_OPERATIONS &_Thread_queue_Operations_priority


     
RTEMS_INLINE_ROUTINE void _DPCP_Acquire_critical(
   DPCP_Control         *dpcp,
   Thread_queue_Context *queue_context
 )
{
  _Thread_queue_Acquire_critical( &dpcp->Wait_queue, queue_context );
}

RTEMS_INLINE_ROUTINE void _DPCP_Release(
  DPCP_Control         *dpcp,
  Thread_queue_Context *queue_context
)
{
 _Thread_queue_Release( &dpcp->Wait_queue, queue_context );
}

RTEMS_INLINE_ROUTINE Thread_Control *_DPCP_Get_owner(
  const DPCP_Control *dpcp
)
{
 return dpcp->Wait_queue.Queue.owner;
}

RTEMS_INLINE_ROUTINE void _DPCP_Set_owner(
  DPCP_Control   *dpcp,
  Thread_Control *owner
)
{
  dpcp->Wait_queue.Queue.owner = owner;
}

RTEMS_INLINE_ROUTINE Priority_Control _DPCP_Get_priority(
  const DPCP_Control      *dpcp,
  const Scheduler_Control *scheduler
)
{
  uint32_t scheduler_index;
  scheduler_index = _Scheduler_Get_index( scheduler );
 return dpcp->ceiling_priorities[ scheduler_index ];
}

 RTEMS_INLINE_ROUTINE void _DPCP_Migrate(
  /**   DPCP_Control         *dpcp,*/
     Thread_Control       *executing,
   /**  Thread_queue_Context *queue_context,*/
    Per_CPU_Control         *cpu
 )
 {

               /** Per_CPU_Control *thread_cpu;
                *       *  thread_cpu = _Thread_Get_CPU( executing );
                *             *   Per_CPU_Control         *cpu = _Per_CPU_Get_by_index( 0 );*/
           /**    _DPCP_Acquire_critical( dpcp, queue_context );*/
 _Thread_Set_CPU(executing,cpu);
/** cpu = _Thread_Dispatch_disable_critical(
      &queue_context->Lock_context.Lock_context
);
 _DPCP_Release( dpcp, queue_context );
 _Thread_Dispatch_enable( cpu );
  /**return STATUS_SUCCESSFUL
      ;*/
 }
RTEMS_INLINE_ROUTINE void _DPCP_Set_priority(
  DPCP_Control            *dpcp,
  const Scheduler_Control *scheduler,
  Priority_Control         new_priority
)
{
  uint32_t scheduler_index;
  scheduler_index = _Scheduler_Get_index( scheduler );
  dpcp->ceiling_priorities[ scheduler_index ] = new_priority;
    }

RTEMS_INLINE_ROUTINE void _DPCP_Replace_priority(
          DPCP_Control   *dpcp,
            Thread_Control *thread,
              Priority_Node  *ceiling_priority
        )
{
      ISR_lock_Context lock_context;

        _Thread_Wait_acquire_default( thread, &lock_context );
          _Thread_Priority_replace( 
                      thread,
                          ceiling_priority,
                              &dpcp->Ceiling_priority
                                );
            _Thread_Wait_release_default( thread, &lock_context );
}



RTEMS_INLINE_ROUTINE Status_Control _DPCP_Raise_priority(
  DPCP_Control         *dpcp,
  Thread_Control       *thread,
  Priority_Node        *priority_node,
 Thread_queue_Context *queue_context
)
 {
   Status_Control           status;
   ISR_lock_Context         lock_context;
   const Scheduler_Control *scheduler;
   Priority_Control         ceiling_priority;
   Scheduler_Node          *scheduler_node;

   _Thread_queue_Context_clear_priority_updates( queue_context );
   _Thread_Wait_acquire_default_critical( thread, &lock_context );
   scheduler = _Thread_Scheduler_get_home( thread );
   scheduler_node = _Thread_Scheduler_get_home_node( thread );
   ceiling_priority = _DPCP_Get_priority( dpcp, scheduler );
       if (
        ceiling_priority 
               < _Priority_Get_priority( &scheduler_node->Wait.Priority )
) {
     _Priority_Node_initialize( priority_node, ceiling_priority );
     _Thread_Priority_add( thread, priority_node, queue_context );
     status = STATUS_SUCCESSFUL;
     } else {
    status = STATUS_MUTEX_CEILING_VIOLATED;
       }
   _Thread_Wait_release_default_critical( thread, &lock_context );
  return status;
    }

RTEMS_INLINE_ROUTINE void _DPCP_Remove_priority( 
  Thread_Control       *thread,
  Priority_Node        *priority_node,
  Thread_queue_Context *queue_context
)
{
  ISR_lock_Context lock_context;
  _Thread_queue_Context_clear_priority_updates( queue_context );
  _Thread_Wait_acquire_default_critical( thread, &lock_context );
  _Thread_Priority_remove( thread, priority_node, queue_context );
  _Thread_Wait_release_default_critical( thread, &lock_context );
}

RTEMS_INLINE_ROUTINE Status_Control _DPCP_Set_new(
  DPCP_Control         *dpcp,
  Thread_Control       *executing,
  Thread_queue_Context *queue_context
)
{
/**    Priority_Control         core_priority;*/
  Status_Control   status;
  Per_CPU_Control *cpu_self;
/**  const Scheduler_Control *scheduler;*/
 /** Per_CPU_Control         *cpu_semaphore = _Per_CPU_Get_by_index( 1 );
  _DPCP_Migrate(executing, cpu_semaphore);
  
 /** scheduler = _Scheduler_Get_by_id(1);

  _Scheduler_Set( scheduler, executing, core_priority );
*/
   status = _DPCP_Raise_priority(
                  dpcp,
                  executing,
                  &dpcp->Ceiling_priority,  
                  queue_context
 );
 if ( status != STATUS_SUCCESSFUL ) { 
    _DPCP_Release( dpcp, queue_context );
   return status;
}
 _DPCP_Set_owner( dpcp, executing );
 cpu_self = _Thread_Dispatch_disable_critical(
                   &queue_context->Lock_context.Lock_context
                        );
  _DPCP_Release( dpcp, queue_context );
  _Thread_Priority_and_sticky_update( executing, 1 );
  _Thread_Dispatch_enable( cpu_self );
  return STATUS_SUCCESSFUL;
    }

RTEMS_INLINE_ROUTINE Status_Control _DPCP_Initialize(
 DPCP_Control            *dpcp,
 const Scheduler_Control *scheduler,
 Priority_Control         ceiling_priority,
 Thread_Control          *executing,
 bool                     initially_locked
)
{
 uint32_t scheduler_count = _Scheduler_Count;
 uint32_t i;
 if ( initially_locked ) {
     return STATUS_INVALID_NUMBER;
 }
dpcp->ceiling_priorities = _Workspace_Allocate(
 sizeof( *dpcp->ceiling_priorities ) * scheduler_count
   );
 if ( dpcp->ceiling_priorities == NULL ) {
  return STATUS_NO_MEMORY;
 }
 for ( i = 0 ; i < scheduler_count ; ++i ) {
         const Scheduler_Control *scheduler_of_index;
         scheduler_of_index = &_Scheduler_Table[ i ];
         if ( scheduler != scheduler_of_index ) {
         dpcp->ceiling_priorities[ i ] =
         _Scheduler_Map_priority( scheduler_of_index, 0 );
         } else {
         dpcp->ceiling_priorities[ i ] = ceiling_priority;
              }
          }
         /**_Thread_queue_Initialize( &dpcp->Wait_queue );*/
  _Thread_queue_Object_initialize( &dpcp->Wait_queue );
         return STATUS_SUCCESSFUL;
    }

RTEMS_INLINE_ROUTINE Status_Control _DPCP_Wait( 
  DPCP_Control            *dpcp,
  const Thread_queue_Operations *operations,
  Thread_Control       *executing,
  Thread_queue_Context          *queue_context
)
{
  Status_Control status;
  Priority_Node  ceiling_priority;

 /**               Per_CPU_Control         *cpu_semaphore = _Per_CPU_Get_by_index(0);

              _DPCP_Migrate(executing, cpu_semaphore);
*/
 status = _DPCP_Raise_priority(
           dpcp,
          executing,
          &ceiling_priority,
          queue_context
);

   if ( status != STATUS_SUCCESSFUL ) {
      _DPCP_Release( dpcp, queue_context );
      return status;
}
  _Thread_queue_Context_set_thread_state(
      queue_context,
      STATES_WAITING_FOR_MUTEX
   );
 _Thread_queue_Context_set_do_nothing_enqueue_callout( queue_context );
 _Thread_queue_Context_set_deadlock_callout(
   queue_context,
 _Thread_queue_Deadlock_status
 );
_Thread_queue_Enqueue( 
 &dpcp->Wait_queue.Queue,
 DPCP_TQ_OPERATIONS,
 executing,
 queue_context
 );
return _Thread_Wait_get_status( executing );
}
/**
RTEMS_INLINE_ROUTINE Status_Control _DPCP_Wait(
          DPCP_Control         *dpcp,
            Thread_Control       *executing,
              Thread_queue_Context *queue_context
        )
{
      Status_Control status;
        Priority_Node  ceiling_priority;

          status = _DPCP_Raise_priority(
                     dpcp,
                          executing,
                              &ceiling_priority,
                                  queue_context
                                    );

            if ( status != STATUS_SUCCESSFUL ) {
                    _DPCP_Release( dpcp, queue_context );
                        return status;
                          }

              _Thread_queue_Context_set_deadlock_callout(
                          queue_context,
                              _Thread_queue_Deadlock_status
                                );
                status = _Thread_queue_Enqueue_sticky(
                            &dpcp->Wait_queue.Queue,
                                DPCP_TQ_OPERATIONS,
                                    executing,
                                        queue_context
                                          );

                  if ( status == STATUS_SUCCESSFUL ) {
                          _DPCP_Replace_priority( dpcp, executing, &ceiling_priority );
                            } else {
                                    Thread_queue_Context  queue_context;
                                        Per_CPU_Control      *cpu_self;
                                            int                   sticky_level_change;

                                                if ( status != STATUS_DEADLOCK ) {
                                                          sticky_level_change = -1;
                                                              } else {
                                                                        sticky_level_change = 0;
                                                                            }

                                                    _ISR_lock_ISR_disable( &queue_context.Lock_context.Lock_context );
                                                        _DPCP_Remove_priority( executing, &ceiling_priority, &queue_context );
                                                            cpu_self = _Thread_Dispatch_disable_critical(
                                                                          &queue_context.Lock_context.Lock_context
                                                                              );
                                                                _ISR_lock_ISR_enable( &queue_context.Lock_context.Lock_context );
                                                                    _Thread_Priority_and_sticky_update( executing, sticky_level_change );
                                                                        _Thread_Dispatch_enable( cpu_self );
                                                                          }

                    return status;
}
*/
RTEMS_INLINE_ROUTINE Status_Control _DPCP_Seize(
  DPCP_Control         *dpcp,
  Thread_Control       *executing,
  bool                  wait,
 Thread_queue_Context *queue_context
)
{
   const Scheduler_Control *scheduler;
 Status_Control  status;
 Thread_Control *owner;
  Scheduler_Node          *scheduler_node;
 Per_CPU_Control         *cpu_semaphore = _Per_CPU_Get_by_index(1);
 _DPCP_Acquire_critical(dpcp, queue_context );
/** scheduler = _Scheduler_Get_by_id( 0 );

      _Scheduler_Set( scheduler,executing,3 );
/** _DPCP_Migrate(executing, cpu_semaphore);
/**scheduler_node = _Thread_Scheduler_get_node_by_index(executing,1);
   _DPCP_Acquire_critical(dpcp, queue_context );*/
 /** _Scheduler_Node_set_user(scheduler_node,executing);*/
    owner = _DPCP_Get_owner(dpcp);
  /** _DPCP_Migrate(owner,cpu_semaphore);*/
    if ( owner == NULL ){
/**   _DPCP_Migrate(owner,cpu_semaphore);*/
        status = _DPCP_Set_new( dpcp, executing, queue_context );
    _DPCP_Migrate(executing, cpu_semaphore);
    
    }
   else if (owner == executing){
    _DPCP_Release (dpcp, queue_context);
    status = STATUS_UNAVAILABLE;
}
   else if ( wait ) {
/**_DPCP_Migrate(owner,cpu_semaphore);*/

 status = _DPCP_Wait(dpcp,executing,DPCP_TQ_OPERATIONS,queue_context);
  _DPCP_Migrate(executing,cpu_semaphore);  

/** _Wait( dpcp, DPCP_TQ_OPERATIONS,executing, queue_context );*/
   } else {
  _DPCP_Release( dpcp, queue_context );
  status = STATUS_UNAVAILABLE;
}
return status;
}

 RTEMS_INLINE_ROUTINE Status_Control _DPCP_Surrender(
                       DPCP_Control         *dpcp,
                Thread_Control       *executing,
                  Thread_queue_Context *queue_context
            )
 {
   Thread_queue_Heads *heads;
   const Scheduler_Control *scheduler;
   Priority_Control         core_priority;
   Scheduler_Node          *scheduler_node;

   Per_CPU_Control         *cpu = _Per_CPU_Get_by_index(0);
/**_DPCP_Migrate(executing,cpu);*/
if ( _DPCP_Get_owner( dpcp ) != executing ) {
        _ISR_lock_ISR_enable( &queue_context->Lock_context.Lock_context );
       return STATUS_NOT_OWNER;
}
  _DPCP_Acquire_critical( dpcp, queue_context );
  _DPCP_Set_owner( dpcp, NULL );
  _DPCP_Remove_priority( executing, &dpcp->Ceiling_priority, queue_context );
   heads = dpcp->Wait_queue.Queue.heads;
_DPCP_Migrate(executing,cpu);
  if ( heads == NULL ) 
{
 Per_CPU_Control *cpu_self;
/**  Per_CPU_Control         *cpu = _Per_CPU_Get_by_index(0);*/
 /**scheduler_node = _Thread_Scheduler_get_home_node( executing );
_Scheduler_Node_set_user(scheduler_node,executing);

/**   _DPCP_Migrate(executing,cpu);
/**  scheduler = _Scheduler_Get_by_id(1);

     _Scheduler_Set( scheduler, executing, 1 );
*/
 cpu_self = _Thread_Dispatch_disable_critical(
        &queue_context->Lock_context.Lock_context
              );
   _DPCP_Release( dpcp, queue_context );
   _Thread_Priority_and_sticky_update( executing, -1 );
   _Thread_Dispatch_enable( cpu_self );
   return STATUS_SUCCESSFUL;
           }
    _Thread_queue_Surrender(
       &dpcp->Wait_queue.Queue,
             heads,
             executing,
            queue_context,
            DPCP_TQ_OPERATIONS
             );

  /**     _Thread_queue_Surrender_sticky( 
                                    &dpcp->Wait_queue.Queue,
                                        heads,
                                            executing,
                                                queue_context,
                                                    DPCP_TQ_OPERATIONS
                                                      );
     */
return STATUS_SUCCESSFUL;
    }

RTEMS_INLINE_ROUTINE Status_Control _DPCP_Can_destroy( DPCP_Control *dpcp )
{
 if ( _DPCP_Get_owner( dpcp ) != NULL ) {
    return STATUS_RESOURCE_IN_USE;
}
 return STATUS_SUCCESSFUL;
}

RTEMS_INLINE_ROUTINE void _DPCP_Destroy(
  DPCP_Control         *dpcp,
  Thread_queue_Context *queue_context
)
{
  _DPCP_Release( dpcp, queue_context );
  _Thread_queue_Destroy( &dpcp->Wait_queue );
  _Workspace_Free( dpcp->ceiling_priorities );
}

    /** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RTEMS_SMP */

#endif /* _RTEMS_SCORE_DPCPIMPL_H */
