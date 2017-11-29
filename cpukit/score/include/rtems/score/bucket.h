struct Bucketsort
{
  uint bitmap;
  Bucket[64] bucket; 
}
struct Bucket
{
int amount_elements;
Watchdog_Control *[64] elements;
}
/* end of include file */
