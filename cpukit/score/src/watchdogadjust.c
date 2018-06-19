/**
 *  @file
 *
 *  @brief Watchdog Adjust
 *  @ingroup ScoreWatchdog
 */

/*
 *  COPYRIGHT (c) 1989-1999.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.org/license/LICENSE.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems/score/watchdogimpl.h>

void _Watchdog_Adjust_backward_locked(
  Watchdog_Header   *header,
  Watchdog_Interval  units
)
{
 _Watchdog_Ticks_since_boot --;
}

void _Watchdog_Adjust_backward(
  Watchdog_Header   *header,
  Watchdog_Interval  units
)
{
  ISR_lock_Context lock_context;

  _Watchdog_Acquire( header, &lock_context );
  _Watchdog_Adjust_backward_locked( header, units );
  _Watchdog_Release( header, &lock_context );
}

void _Watchdog_Adjust_forward_locked(
  Watchdog_Header   *header,
  Watchdog_Interval  units,
  ISR_lock_Context  *lock_context
)
{
 _Watchdog_Ticks_since_boot ++; 
}

void _Watchdog_Adjust_forward(
  Watchdog_Header   *header,
  Watchdog_Interval  units
)
{
  ISR_lock_Context lock_context;

  _Watchdog_Acquire( header, &lock_context );
  _Watchdog_Adjust_forward_locked( header, units, &lock_context );
  _Watchdog_Release( header, &lock_context );
}
