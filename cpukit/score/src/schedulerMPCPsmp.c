#if HAVE_CONFIG_H
  #include "config.h"
#endif
#include <rtems/score/schedulerMPCPsmp.h>
#include <rtems/score/schedulerpriorityimpl.h>
#include <rtems/score/schedulersmpimpl.h>
#include <rtems/score/schedulerprioritysmpimpl.h>
#include <rtems/score/wkspace.h>
#include <rtems/score/cpusetimpl.h>
#include <rtems/score/priority.h>

/*
 *  * The following methods which initially were static in schedulerprioritysmp.c
 *   * are shared with this scheduler. 
 * 
 *  + _Scheduler_priority_SMP_Get_self
 *  + _Scheduler_priority_SMP_Insert_ready_fifo
 *  + _Scheduler_priority_SMP_Insert_ready_lifo
 *  + _Scheduler_priority_SMP_Thread_get_node
 *  + _Scheduler_priority_SMP_Move_from_scheduled_to_ready
 *  + _Scheduler_priority_SMP_Move_from_ready_to_scheduled
 *  + _Scheduler_priority_SMP_Extract_from_ready
 *  + _Scheduler_priority_SMP_Do_update
 *             */


void _Scheduler_MPCP_SMP_Node_initialize(
   const Scheduler_Control *scheduler,
         Scheduler_Node          *node,
         Thread_Control          *the_thread,
         Priority_Control         priority
        )
{
      Scheduler_Context              *context;
      Scheduler_priority_SMP_Context *self;
      Scheduler_priority_SMP_Node    *the_node;
      the_node = _Scheduler_priority_SMP_Node_downcast( node );
      _Scheduler_SMP_Node_initialize(
        scheduler,
        &the_node->Base,
        the_thread,
        priority
           );
      context = _Scheduler_Get_context( scheduler );
      self = _Scheduler_priority_SMP_Get_self( context );
      _Scheduler_priority_Ready_queue_update(
      &the_node->Ready_queue,
      priority,
      &self->Bit_map,
      &self->Ready[ 0 ]
           );
} 

static bool _Scheduler_MPCP_SMP_Insert_priority_lifo_order(
       const Chain_Node *to_insert,
       const Chain_Node *next
        )
{
      return next != NULL
            && _Scheduler_SMP_Insert_priority_lifo_order( to_insert, next );
}

static bool _Scheduler_MPCP_SMP_Insert_priority_fifo_order(
        const Chain_Node *to_insert,
        const Chain_Node *next
        )
{
      return next != NULL
              && _Scheduler_SMP_Insert_priority_fifo_order( to_insert, next );
}

/*
 *  * This method returns the scheduler node for the specified thread
 *   * as a scheduler specific type.
 *    */
static  Scheduler_Node *_Scheduler_MPCP_SMP_Get_highest_ready(
          Scheduler_Context *context,
            Scheduler_Node    *node
        )
{
      Scheduler_priority_SMP_Context *self =
     _Scheduler_priority_SMP_Get_self( context );
     (void) node;
     return (Scheduler_Node *) _Scheduler_priority_Ready_queue_first(
            &self->Bit_map,
            &self->Ready[ 0 ]
                 );
}


        /** static void _Scheduler_MPCP_Allocate_processor_lazy(
                      Scheduler_Context *context,
                        Thread_Control    *scheduled_thread,
                          Thread_Control    *victim_thread,
                            Per_CPU_Control   *victim_cpu
                    )
            {
                  Per_CPU_Control *scheduled_cpu = _Thread_Get_CPU( scheduled_thread );
                    Per_CPU_Control *cpu_self = _Per_CPU_Get();
                      Thread_Control *heir;
                      (void) victim_cpu;
                        _Assert( _ISR_Get_level() != 0 );
                          

                         if ( _Thread_Is_executing_on_a_processor( scheduled_thread ) ) {  
                              
                                   heir = scheduled_cpu->heir;
                                         _Thread_Dispatch_update_heir( 
                                                         cpu_self,
                                                                 scheduled_cpu,
                                                                         scheduled_thread
                                                                               ); 
                                           } 
                          
                          else {
                                 heir = scheduled_thread;
                                    }

                             if ( heir != victim_thread ) { 
                                    _Thread_Set_CPU( heir, scheduled_cpu); 
                                         _Thread_Dispatch_update_heir( cpu_self, scheduled_cpu, heir ); 
                                            }
            }

           */  
/** This method blocks the thread */            
void _Scheduler_MPCP_SMP_Block(
      const Scheduler_Control *scheduler,
            Thread_Control          *thread,
            Scheduler_Node          *node
                    )
            {
  Scheduler_Context *context = _Scheduler_Get_context( scheduler );
  _Scheduler_SMP_Block(
         context,
         thread,
         node,
        _Scheduler_priority_SMP_Extract_from_ready,
        _Scheduler_MPCP_SMP_Get_highest_ready,
        _Scheduler_priority_SMP_Move_from_ready_to_scheduled,
       _Scheduler_SMP_Allocate_processor_lazy
           );
                
          }
 static bool _Scheduler_MPCP_SMP_Enqueue_fifo(
            Scheduler_Context *context,
           Scheduler_Node    *node
       )
  {
  return _Scheduler_SMP_Enqueue_ordered(
         context,
         node,
        _Scheduler_MPCP_SMP_Insert_priority_fifo_order,
      _Scheduler_priority_SMP_Insert_ready_fifo,
      _Scheduler_SMP_Insert_scheduled_fifo,
      _Scheduler_priority_SMP_Move_from_scheduled_to_ready,
      _Scheduler_SMP_Get_lowest_scheduled,
     _Scheduler_SMP_Allocate_processor_lazy
           );
      }

/** static bool _Scheduler_MPCP_SMP_Enqueue_fifo(
            Scheduler_Context *context,
               Scheduler_Node    *node
                )
     {
            return _Scheduler_MPCP_SMP_Enqueue_ordered(
                         context,
                              node,
                                   _Scheduler_MPCP_SMP_Insert_priority_fifo_order,
                                        _Scheduler_priority_SMP_Insert_ready_fifo,
                                        
                                             _Scheduler_SMP_Insert_scheduled_fifo
                                                );
             }*/
/** this method unblock the corresponding thread*/                
void _Scheduler_MPCP_SMP_Unblock(
   const Scheduler_Control *scheduler,
         Thread_Control          *thread,
        Scheduler_Node          *node
        )
        {
  
  Scheduler_Context *context = _Scheduler_Get_context( scheduler );
  _Scheduler_SMP_Unblock(
                        context,
                        thread,
                        node,
                       _Scheduler_priority_SMP_Do_update,
                       _Scheduler_MPCP_SMP_Enqueue_fifo
                        );
                    }

static bool _Scheduler_MPCP_SMP_Enqueue_ordered(
          Scheduler_Context     *context,
          Scheduler_Node        *node,
          Chain_Node_order       order,
          Scheduler_SMP_Insert   insert_ready,
          Scheduler_SMP_Insert   insert_scheduled
      )
         {
   return _Scheduler_SMP_Enqueue_ordered(
          context,
          node,
          order,
          insert_ready,
          insert_scheduled,
         _Scheduler_priority_SMP_Move_from_scheduled_to_ready,
         _Scheduler_SMP_Get_lowest_scheduled,
         _Scheduler_SMP_Allocate_processor_lazy
      );
           }

static bool _Scheduler_MPCP_SMP_Enqueue_lifo(
            Scheduler_Context *context,
            Scheduler_Node    *node
         )
   {
 return _Scheduler_MPCP_SMP_Enqueue_ordered(
        context,
        node,
       _Scheduler_MPCP_SMP_Insert_priority_lifo_order,
       _Scheduler_priority_SMP_Insert_ready_lifo,
       _Scheduler_SMP_Insert_scheduled_lifo
               );
   }

static bool _Scheduler_MPCP_SMP_Enqueue_scheduled_ordered(
            Scheduler_Context    *context,
            Scheduler_Node       *node,
            Chain_Node_order      order,
            Scheduler_SMP_Insert  insert_ready,
            Scheduler_SMP_Insert  insert_scheduled,
              MPCP_Control *mpcp )
      {
     return _Scheduler_SMP_Enqueue_scheduled_ordered_MPCP
         (
              context,
              node,
              order,
              _Scheduler_priority_SMP_Extract_from_ready,
              _Scheduler_MPCP_SMP_Get_highest_ready,
              insert_ready,
              insert_scheduled,
             _Scheduler_priority_SMP_Move_from_ready_to_scheduled,
             _Scheduler_SMP_Allocate_processor_lazy,
     mpcp
             );
                             }

 static bool _Scheduler_MPCP_SMP_Enqueue_scheduled_lifo(
         Scheduler_Context *context,
         Scheduler_Node    *node,
         MPCP_Control *mpcp        )
    {
    return _Scheduler_MPCP_SMP_Enqueue_scheduled_ordered(
           context,
           node,
          _Scheduler_SMP_Insert_priority_lifo_order,
          _Scheduler_priority_SMP_Insert_ready_lifo,
          _Scheduler_SMP_Insert_scheduled_lifo,
           mpcp    );
                             }

static bool _Scheduler_MPCP_SMP_Enqueue_scheduled_fifo(
            Scheduler_Context *context,
           Scheduler_Node    *node,
           MPCP_Control *mpcp
             )
                             {
  return _Scheduler_MPCP_SMP_Enqueue_scheduled_ordered(
         context,
         node,
        _Scheduler_SMP_Insert_priority_fifo_order,
        _Scheduler_priority_SMP_Insert_ready_fifo,
        _Scheduler_SMP_Insert_scheduled_fifo,
          mpcp    );
                             }
 static bool _Scheduler_MPCP_SMP_Do_ask_for_help(
            Scheduler_Context *context,
               Thread_Control    *the_thread,
                  Scheduler_Node    *node
                   )
     {
            return _Scheduler_SMP_Ask_for_help(
                         context,
                              the_thread,
                                   node,
                                        _Scheduler_SMP_Insert_priority_lifo_order,
                                             _Scheduler_priority_SMP_Insert_ready_lifo,
                                             
                                                  _Scheduler_SMP_Insert_scheduled_lifo,
                                                       _Scheduler_priority_SMP_Move_from_scheduled_to_ready,
                                    
                                                            _Scheduler_SMP_Get_lowest_scheduled,
                                                                 _Scheduler_SMP_Allocate_processor_lazy
                                                                    );
             }

/**this function updates a priority of the thread*/
 void _Scheduler_MPCP_SMP_Update_priority(
     const Scheduler_Control *scheduler,
     Thread_Control          *thread,
     Scheduler_Node          *node
               )
    {
 Scheduler_Context *context = _Scheduler_Get_context( scheduler );
 _Scheduler_SMP_Update_priority(
  context,
  thread,
  node,
  _Scheduler_priority_SMP_Extract_from_ready,
  _Scheduler_priority_SMP_Do_update,
  _Scheduler_MPCP_SMP_Enqueue_fifo,
  _Scheduler_MPCP_SMP_Enqueue_lifo,
  _Scheduler_MPCP_SMP_Enqueue_scheduled_fifo,
  _Scheduler_MPCP_SMP_Enqueue_scheduled_lifo,
_Scheduler_MPCP_SMP_Do_ask_for_help
  /** _Scheduler_SMP_Ask_for_help*/
 /** _Scheduler_default_Ask_for_help*/
     /**     false */
  /**_Scheduler_MPCP_SMP_Do_ask_for_help
                */ );
      }
             
  void _Scheduler_MPCP_SMP_Withdraw_node(
     const Scheduler_Control *scheduler,
     Thread_Control          *the_thread,
     Scheduler_Node          *node,
     Thread_Scheduler_state   next_state
       )
   {
   Scheduler_Context *context = _Scheduler_Get_context( scheduler );
   _Scheduler_SMP_Withdraw_node(
   context,
  the_thread,
  node,
  next_state,
 _Scheduler_priority_SMP_Extract_from_ready,
 _Scheduler_MPCP_SMP_Get_highest_ready,
 _Scheduler_priority_SMP_Move_from_ready_to_scheduled,
 _Scheduler_SMP_Allocate_processor_lazy
                );
                             }

  void _Scheduler_MPCP_SMP_Add_processor(
     const Scheduler_Control *scheduler,
     Thread_Control          *idle
             )
  {
   Scheduler_Context *context = _Scheduler_Get_context( scheduler );
  _Scheduler_SMP_Add_processor(
   context,
   idle,
  _Scheduler_priority_SMP_Has_ready,
 _Scheduler_MPCP_SMP_Enqueue_scheduled_fifo
         );
          }
  
Thread_Control *_Scheduler_MPCP_SMP_Remove_processor(
     const Scheduler_Control *scheduler,
     Per_CPU_Control         *cpu
          )
{
  Scheduler_Context *context = _Scheduler_Get_context( scheduler );
   return _Scheduler_SMP_Remove_processor(
          context,
          cpu,
          _Scheduler_priority_SMP_Extract_from_ready,
         _Scheduler_MPCP_SMP_Enqueue_fifo
               );
            }

