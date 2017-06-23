#ifndef _RTEMS_SCORE_MPCP_H
#define _RTEMS_SCORE_MPCP_H

#include <rtems/score/cpuopts.h>

#if defined(RTEMS_SMP)

#include <rtems/score/threadq.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
  /**
   *  *  MPCP control block.
   *   */
  typedef struct {
  /**
   **  The thread queue to manage ownership and waiting threads.
   **/
   Thread_queue_Control Wait_queue;
   /**
    * * The ceiling priority used by the owner thread.
    **/
   Priority_Node Ceiling_priority;
    /**
     **  One ceiling priority per scheduler instance.
    **/
   Priority_Control *ceiling_priorities;
  } MPCP_Control;

  /** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RTEMS_SMP */

#endif /* _RTEMS_SCORE_MPCP_H */
