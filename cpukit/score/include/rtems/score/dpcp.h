#ifndef _RTEMS_SCORE_DPCP_H
#define _RTEMS_SCORE_DPCP_H

#include <rtems/score/cpuopts.h>

#if defined(RTEMS_SMP)

#include <rtems/score/threadq.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 @brief DPCP control block.
 */
typedef struct {
 /**
 @brief The thread queue to manage ownership and waiting threads.
  */
Thread_queue_Control Wait_queue;
/**
 @brief The ceiling priority used by the owner thread.
  */
 Priority_Node Ceiling_priority;
 /**
 @brief One ceiling priority per scheduler instance.
    */
Priority_Control *ceiling_priorities;
         
} DPCP_Control;
    /** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RTEMS_SMP */

#endif /* _RTEMS_SCORE_DPCP_H */
