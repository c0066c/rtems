#ifndef _RTEMS_SCROE_BUCKET_H
#define _RTEMS_SCORE_BUCKET_H
typedef struct
{

int amount_elements;
Watchdog_Control  elements[64];
}Bucket;

#endif
/* end of include file */
