#ifndef _RTEMS_SCORE_DNPPIMPL_H
#define _RTEMS_SCORE_DNPPIMPL_H

#include <rtems/score/dnpp.h>

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
     *  * @addtogroup ScoreDNPP
     *   *
     *    * @{
     *     */
#define DNPP_TQ_OPERATIONS &_Thread_queue_Operations_priority
     
RTEMS_INLINE_ROUTINE void _DNPP_Acquire_critical(
   DNPP_Control         *dnpp,
   Thread_queue_Context *queue_context
 )
{
  _Thread_queue_Acquire_critical( &dnpp->Wait_queue, queue_context );
}

RTEMS_INLINE_ROUTINE void _DNPP_Release(
  DNPP_Control         *dnpp,
  Thread_queue_Context *queue_context
)
{
 _Thread_queue_Release( &dnpp->Wait_queue, queue_context );
}

RTEMS_INLINE_ROUTINE Thread_Control *_DNPP_Get_owner(
  const DNPP_Control *dnpp
)
{
 return dnpp->Wait_queue.Queue.owner;
}

 RTEMS_INLINE_ROUTINE void _DNPP_Set_Ceiling (
   DNPP_Control *dnpp ,
   const Scheduler_Control *scheduler
)
{
  uint32_t scheduler_index ;

  scheduler_index = _Scheduler_Get_index ( scheduler ) ;
  dnpp ->ceiling_priorities [ scheduler_index ] = 1 ;
} 


RTEMS_INLINE_ROUTINE void _DNPP_Set_owner(
  DNPP_Control   *dnpp,
  Thread_Control *owner
)
{
  dnpp->Wait_queue.Queue.owner = owner;
}

RTEMS_INLINE_ROUTINE Priority_Control _DNPP_Get_priority(
  const DNPP_Control      *dnpp,
  const Scheduler_Control *scheduler
)
{
  uint32_t scheduler_index;
  scheduler_index = _Scheduler_Get_index( scheduler );

  return dnpp->ceiling_priorities[ scheduler_index ];
}
/**tasks migration function*/
 RTEMS_INLINE_ROUTINE void _DNPP_Migrate(
     Thread_Control       *executing,
    Per_CPU_Control         *cpu
 )
 {
 _Thread_Set_CPU(executing,cpu);
 }

RTEMS_INLINE_ROUTINE void _DNPP_Set_priority(
  DNPP_Control            *dnpp,
  const Scheduler_Control *scheduler,
  Priority_Control         new_priority
)
{
  uint32_t scheduler_index;
  scheduler_index = _Scheduler_Get_index( scheduler );
  dnpp->ceiling_priorities[ scheduler_index ] = new_priority;
    }

RTEMS_INLINE_ROUTINE void _DNPP_Replace_priority(
      DNPP_Control   *dnpp,
      Thread_Control *thread,
      Priority_Node  *ceiling_priority
)
{
 ISR_lock_Context lock_context;

 _Thread_Wait_acquire_default( thread, &lock_context );
 _Thread_Priority_replace( 
        thread,
        ceiling_priority,
        &dnpp->Ceiling_priority
);

 _Thread_Wait_release_default( thread, &lock_context );
}
/**raise priority to ceilings*/
RTEMS_INLINE_ROUTINE Status_Control _DNPP_Raise_priority(
  DNPP_Control         *dnpp,
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
 ceiling_priority = _DNPP_Get_priority( dnpp, scheduler );
 
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

RTEMS_INLINE_ROUTINE void _DNPP_Remove_priority( 
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
/**set new owner of resource*/
RTEMS_INLINE_ROUTINE Status_Control _DNPP_Set_new(
  DNPP_Control         *dnpp,
  Thread_Control       *executing,
  Thread_queue_Context *queue_context
)
{
  Status_Control   status;
  Per_CPU_Control *cpu_self;
  status = _DNPP_Raise_priority(
                  dnpp,
                  executing,
                  &dnpp->Ceiling_priority,  
                  queue_context
 );
 if ( status != STATUS_SUCCESSFUL ) { 
    _DNPP_Release( dnpp, queue_context );
   return status;
}
 _DNPP_Set_owner( dnpp, executing );
 cpu_self = _Thread_Dispatch_disable_critical(
            &queue_context->Lock_context.Lock_context
);
  _DNPP_Release( dnpp, queue_context );
  _Thread_Priority_and_sticky_update( executing, 1 );
  _Thread_Dispatch_enable( cpu_self );
  return STATUS_SUCCESSFUL;
    }

RTEMS_INLINE_ROUTINE Status_Control _DNPP_Initialize(
 DNPP_Control            *dnpp,
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
 dnpp->ceiling_priorities = _Workspace_Allocate(
 sizeof( *dnpp->ceiling_priorities ) * scheduler_count);
 
 if ( dnpp->ceiling_priorities == NULL ) {
  return STATUS_NO_MEMORY;
 }
 for ( i = 0 ; i < scheduler_count ; ++i ) {
         const Scheduler_Control *scheduler_of_index;
         scheduler_of_index = &_Scheduler_Table[ i ];
         if ( scheduler != scheduler_of_index ) {
         dnpp->ceiling_priorities[ i ] =
         _Scheduler_Map_priority( scheduler_of_index, 0 );
         } else {
         dnpp->ceiling_priorities[ i ] = ceiling_priority;
              }
          }
  _Thread_queue_Object_initialize( &dnpp->Wait_queue );
         return STATUS_SUCCESSFUL;
    }
/suspending in waiting queue*/
RTEMS_INLINE_ROUTINE Status_Control _DNPP_Wait( 
  DNPP_Control            *dnpp,
  const Thread_queue_Operations *operations,
  Thread_Control       *executing,
  Thread_queue_Context          *queue_context
)
{
  Status_Control status;
  Priority_Node  ceiling_priority;

  status = _DNPP_Raise_priority(
           dnpp,
           executing,
           &ceiling_priority,
           queue_context
);

 if ( status != STATUS_SUCCESSFUL ) {
     _DNPP_Release( dnpp, queue_context );
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
 &dnpp->Wait_queue.Queue,
 DNPP_TQ_OPERATIONS,
 executing,
 queue_context
 );

 return _Thread_Wait_get_status( executing );
}
/**obtain semaphore function*/
RTEMS_INLINE_ROUTINE Status_Control _DNPP_Seize(
  DNPP_Control         *dnpp,
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
 
 _DNPP_Acquire_critical(dnpp, queue_context );
 scheduler = _Thread_Scheduler_get_home( executing );
 owner = _DNPP_Get_owner(dnpp);
 
 _DNPP_Set_Ceiling ( dnpp,scheduler);
    if ( owner == NULL ){
        status = _DNPP_Set_new( dnpp, executing, queue_context );
        _DNPP_Migrate(executing, cpu_semaphore);
    }
   else if (owner == executing){
 
   _DNPP_Release (dnpp, queue_context);
   status = STATUS_UNAVAILABLE;
}
   else if ( wait ) {
 status = _DNPP_Wait(dnpp,executing,DNPP_TQ_OPERATIONS,queue_context);
_DNPP_Migrate(executing,cpu_semaphore);  
   } else {
 
  _DNPP_Release( dnpp, queue_context );
  status = STATUS_UNAVAILABLE;
}
return status;
}
/**release semaphore function*/
RTEMS_INLINE_ROUTINE Status_Control _DNPP_Surrender(
   DNPP_Control         *dnpp,
   Thread_Control       *executing,
   Thread_queue_Context *queue_context
)
{
   Thread_queue_Heads *heads;
   const Scheduler_Control *scheduler;
   Priority_Control         core_priority;
   Scheduler_Node          *scheduler_node;

   Per_CPU_Control         *cpu = _Per_CPU_Get_by_index(0);

   if ( _DNPP_Get_owner( dnpp ) != executing ) {
        _ISR_lock_ISR_enable( &queue_context->Lock_context.Lock_context );
       return STATUS_NOT_OWNER;
}
  _DNPP_Acquire_critical( dnpp, queue_context );
  _DNPP_Set_owner( dnpp, NULL );
  _DNPP_Remove_priority( executing, &dnpp->Ceiling_priority, queue_context );
   heads = dnpp->Wait_queue.Queue.heads;
_DNPP_Migrate(executing,cpu);
  if ( heads == NULL ) 
{
 Per_CPU_Control *cpu_self;
 cpu_self = _Thread_Dispatch_disable_critical(
     &queue_context->Lock_context.Lock_context
              );
   _DNPP_Release( dnpp, queue_context );
   _Thread_Priority_and_sticky_update( executing, -1 );
   _Thread_Dispatch_enable( cpu_self );
   return STATUS_SUCCESSFUL;
           }
    _Thread_queue_Surrender(
       &dnpp->Wait_queue.Queue,
             heads,
             executing,
            queue_context,
            DNPP_TQ_OPERATIONS
             );
return STATUS_SUCCESSFUL;
    }

RTEMS_INLINE_ROUTINE Status_Control _DNPP_Can_destroy( DNPP_Control *dnpp )
{
 if ( _DNPP_Get_owner( dnpp ) != NULL ) {
    return STATUS_RESOURCE_IN_USE;
}
 return STATUS_SUCCESSFUL;
}

RTEMS_INLINE_ROUTINE void _DNPP_Destroy(
  DNPP_Control         *dnpp,
  Thread_queue_Context *queue_context
)
{
  _DNPP_Release( dnpp, queue_context );
  _Thread_queue_Destroy( &dnpp->Wait_queue );
  _Workspace_Free( dnpp->ceiling_priorities );
}

    /** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RTEMS_SMP */

#endif /* _RTEMS_SCORE_DNPPIMPL_H */
