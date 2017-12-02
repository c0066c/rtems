#ifndef _RTEMS_SCORE_BUCKET_H
#define _RTEMS_SCORE_BUCKET_H

#include <rtems/score/watchdog.h>
#include <rtems/score/watchdogimpl.h>

typedef struct
{

int amount_elements;
Watchdog_Control  elements[64];
}Bucket;

#endif
/* end of include file */
