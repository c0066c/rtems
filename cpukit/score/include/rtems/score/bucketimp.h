#include <limits.h>
#include <bucket.h>
Bucketsort buckets;


void _Bucket_Initialize(
  Bucket_Control *the_bucket,
  void          *starting_address
)
{
	
}

void _Bucket_insert(uint64_t position, watchdog the_watchdog)
{
  // insert;
  bucket[position].amount_elements++;
  bucket[position].elements[bucket[position].amount_elements] = the_watchdog;

  //updating bitmap
  
 bitmap= bitmap ||2^position;
  
}

void _Bucket_Remove_Scheduled()
 {
 buckets.bucket[scheduled_position].amount_elements—-;
 if(buckets.bucket[scheduled_position].amount_elements==0)
 {
	int max=INT_MAX;
	buckets.bitmap=	max-2^(scheduled_position)&& bucketsort.bitmap;	
 }
 }
/* end of include file */
