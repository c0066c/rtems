#ifndef _RTEMS_SCORE_BUCKETIMPL_H
#define _RTEMS_SCROE_BUCKETIMPL_H


#include <limits.h>
#include <rtems/score/bucket.h>
#include <rtems/score/watchdog.h>
#include <rtems/score/watchdogimpl.h>

#include <math.h>

void _Bucket_instert(uint64_t position, Watchdog_Control the_watchdog);
void _Bucket_remove_scheduled();
Watchdog_Control _Bucket_next_first();
void _Bucket_Initialize_empty();

uint64_t bitmap;
Bucket buckets[64];
int scheduled_position;

void _Bucket_Initialize_empty()
{
  bitmap=0;
  scheduled_position=0;
}
void _Bucket_insert(uint64_t position, Watchdog_Control  the_watchdog)
{
      // insert;
         buckets[position].amount_elements++;
           buckets[position].elements[buckets[position].amount_elements] = the_watchdog;
      
             //updating bitmap
               
                bitmap= bitmap ||2^position;
               
                  }
    
        void _Bucket_remove_scheduled()
                   {
                    buckets[scheduled_position].amount_elements--;
                     if(buckets[scheduled_position].amount_elements==0)
                      {
                      int max=INT_MAX;
                      bitmap=max-(2^(scheduled_position))&& bitmap;
                       }
                        }

Watchdog_Control _Bucket_next_first()
{
  int tick = _Watchdog_Ticks_since_boot;
    tick = tick%64;
    int high_priority = bitmap%tick;
    int low_priority = bitmap/tick;
    if(high_priority != 0)
    {
      int next = log(high_priority);
      int amount=buckets[next].amount_elements;
      scheduled_position=next;
      return buckets[next].elements[amount];
    } else {
      int next = log(low_priority);
      int amount=buckets[next].amount_elements;
      scheduled_position=next;
      return buckets[next].elements[amount];
    }

#endif
                      /* end of include file */
