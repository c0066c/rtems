typedef struct
{

int amount_elements;
Watchdog_Control* elements[64];
}Bucket;


typedef struct
{
      uint bitmap;
        Bucket bucket[64]; 
} Bucketsort;

/* end of include file */
