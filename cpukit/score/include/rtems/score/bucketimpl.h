  /* This file implements the  operations for the buckets, including: initilisation, insertion and removing
   * And it includes the following logical opertion: Next first
   * It contains also an array that represents the chain of buckets
   */


#ifndef _RTEMS_SCORE_BUCKETIMPL_H
#define _RTEMS_SCORE_BUCKETIMPL_H




  /* defines the Size of the array */
  #define SIZE 3000
  #define GRANULARITY 1
  #define NOGRANULARITY
  #include <rtems/score/object.h>
  #include <limits.h>
  #include <math.h>
  #include <rtems/score/assert.h>
  #include <rtems/score/isrlock.h>
  #include <rtems/score/isr.h>
  #include <rtems/score/chainimpl.h>
  #include <rtems/score/bucket.h>
  #include <rtems/score/watchdog.h>
  #include <rtems/score/watchdogimpl.h>
  
  /* Doubly Linked List implementation */
  #include<stdio.h>
  #include<stdlib.h>

  volatile struct Element  {
	volatile Watchdog_Control* data;
	struct Element* next;
	volatile struct Element* prev;
  } typedef Element_struct;

 volatile struct Bucket_Interval{
     volatile uint32_t sema;
 };
  volatile struct Element* head[SIZE]; // global variable - pointer to head node.
  volatile struct Bucket_Interval bucketsema;
  //struct Element* reference;
  //Creates a new Element and returns pointer to it. 
  RTEMS_INLINE_ROUTINE struct Element GetNewElement(Watchdog_Control *x) {	
      struct Element newElement;
	newElement.data=NULL;
        newElement.data = x;
	newElement.prev = NULL;
	newElement.next = NULL;
        //printk("newElement next: %x \n",newElement.next);
	return newElement;
  }

  //Inserts a Node at head of doubly linked list
  RTEMS_INLINE_ROUTINE struct Element InsertAtHead(Watchdog_Control* x, volatile uint32_t bucket) {
     //while(bucketsema.sema<=0);
      //bucketsema.sema--;
      //printk(" \n 1 \n");
      //printk("%x , %d\n",head[bucket], bucket);
      struct Element newElement = GetNewElement(x);
      //printk("directy after creation: %x \n",head[bucket]->next);
      //printk("Data after insert: %x",head[bucket]->data);
	if(head[bucket] == NULL) {	
             head[bucket] =&newElement;
            // printk("%x , %d\n",head[bucket], bucket);
            // printk("here next: %x \n",head[bucket]->next);
		return newElement;
	}
       // printk("danger asdasdasd 1.13!!!!!!!!!!!!!!!!!");
	head[bucket]->prev = &newElement;
	newElement.next = head[bucket]; 
	head[bucket] = &newElement;
        return newElement;
  }

 RTEMS_INLINE_ROUTINE   Watchdog_Control*  RemoveHead(volatile uint32_t bucket){
                volatile struct Element first = *head[bucket];
                volatile uint32_t bucketlocal=bucket;
               Watchdog_Control*  value=NULL;
		if(first.next != NULL)
		{
                    volatile struct Element* current=head[bucketlocal];
                    if(current==&first)
                    {
                        head[bucketlocal]=first.next;
                    }
                    else
                    {
                        head[bucketlocal]=NULL;
                    }
		}
                else
                {
                    head[bucketlocal]=NULL;
                }
                if(first.next==head[bucketlocal]||first.next==first.next->next)
                {
                    value=first.data;
                }
                bucketlocal++;
	return value;
  }
  
 RTEMS_INLINE_ROUTINE  void RemoveElement (struct Element* x, volatile uint32_t bucket){
        //while(bucketsema.sema <=0);
        //bucketsema.sema--;
	if(x==head[bucket])
	{
		head[bucket] = x->next;
	}
	if(x->next != NULL && x-> prev!= NULL)
	{
		x->prev->next = x->next;
		x->next->prev = x->prev;
	}
	else if(x->prev == NULL && x->next != NULL)
	{
		x->next->prev = NULL;
	}
	else if(x->prev != NULL && x->next == NULL)
	{
		x->prev->next = NULL; 
	}
        //bucketsema.sema++;
  }

RTEMS_INLINE_ROUTINE void _Bucket_Initialize()
{
 //reference=GetNewElement(NULL);
 int i=0;
 for( i=0; i<SIZE;i++)
 {
    head[i]=NULL;
 }
 bucketsema.sema=1;
}

RTEMS_INLINE_ROUTINE bool _bucket_is_empty(volatile uint32_t bucket)
{   
    if(!head[bucket])
    {
     return true;
    }
    return false;
}
RTEMS_INLINE_ROUTINE int Get_Bucket_Size()
{
    int i=SIZE;
 return i;
}

RTEMS_INLINE_ROUTINE int _Get_Granularity()
{
    return GRANULARITY;
}
#endif
    /* end of include file*/ 
