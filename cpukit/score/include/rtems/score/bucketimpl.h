  /* This file implements the  operations for the buckets, including: initilisation, insertion and removing
   * And it includes the following logical opertion: Next first
   * It contains also an array that represents the chain of buckets
   */


  #ifndef _RTEMS_SCORE_BUCKETIMPL_H
  #define _RTEMS_SCORE_BUCKETIMPL_H




  /* defines the Size of the array */
  #define SIZE 32

  #include <limits.h>
  #include <math.h>

  #include <rtems/score/bucket.h>
  #include <rtems/score/watchdog.h>
  #include <rtems/score/watchdogimpl.h>


  unsigned int bitmap;
  Bucket bucket[SIZE];

  /* Initializition for the buckets and the array */
  void _Bucket_Initialize_Empty();


  /* inserts a Watchdog_Control at the given number from the corresponding bucket*/
  void _Bucket_Insert(unsigned int number_bucket, Watchdog_Control the_watchdog);


  /* removes the given watchdog from the bucket */
  void _Bucket_Remove(Watchdog_Control the_watchdog);


  /*identifies the the closest Watchdog_Control to the current amount of system ticks */
  Watchdog_Control _Bucket_next_first();



  void _Bucket_Initialize_Empty();
  {
    bitmap = 0;
  }


  void _Bucket_Insert( unsigned int number_bucket, Watchdog_Control the_watchdog)
  {
    /* updating bitmap */ 
    if(bucket[number.bucket].amount_elements==0)
    {
        bitmap = bitmap + 2^number_bucket;
    }
    bucket[number_bucket].amount_elements ++;
    /* insertion to bucket */
    if(bucket[number_bucket].first == NULL)
    {
        bucket[number_bucket]=the_watchdog;
    }
    else
    {
        bucket[number_bucket].first.previous=the_watchdog;
        the_watchdog.next = bucket[number_bucket].first;
        bucket[number_bucket].first = the_watchdog;
    }
    
  }  

  
  void _Bucket_Remove(Watchdog_Control the_watchdog)
  {
    /* remove from bucket */
    int number_bucket=the_watchdog.location;
    int position_in_bucke t= bucket[number_bucket].amount_elements;
    bucket[number_bucket].amount_elements--;

    /* updating Bitmap and rearange remaining watchdogs */
    if(bucket[number_bucket].amount_elements==0)
    {
        bitmap = bitmap - 2^number_bucket;
        bucket.first=NULL;
    }
    else 
    {
        if(the_watchdog.next!=NULL && the_watchdog.previous!=NULL)
        {
        the_watchdog.next = the_watchdog.previous;
        the_watchdog.previous = the_watchdog.next;
        } else if(the_watchdog.next == NULL)
          {
            the_watchdog.previous.next = NULL;
          }
          else
          {
            the_watchdog.next.previous  = NULL;
          }

    }
  }

  Watchdog_Control _Bucket_next_first()
  {
    /* empty bitmap --> no watchdog waiting */
    if (bitmap==0)
    {
        return NULL;
    }

    /* split bitmap in a close and a far part to compute closest not empty bucket */
    int pointer = System_ticks_since_boot()%(SIZE-1)  
    int lower_bits = (SIZE-1)%2^pointer;
    
    if(lower_bits>0)
    {
        return bucket[(log(lower_bits)/log(2))].first;
    }
    else
    {
        int higher_bits = bitmap/2^pointer;
        return bucket[(log(higher_bits/log(2)))].first();
    }
  }

  #endif
  /* end of include file*/ 
