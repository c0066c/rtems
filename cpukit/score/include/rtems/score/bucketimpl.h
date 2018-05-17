  /* This file implements the  operations for the buckets, including: initilisation, insertion and removing
   * And it includes the following logical opertion: Next first
   * It contains also an array that represents the chain of buckets
   */


#ifndef _RTEMS_SCORE_BUCKETIMPL_H
#define _RTEMS_SCORE_BUCKETIMPL_H




  /* defines the Size of the array */
  #define SIZE 3000
  #include <rtems/score/object.h>
  #include <limits.h>
  #include <math.h>
  #include <rtems/score/assert.h>
  #include <rtems/score/isrlock.h>
  #include <rtems/score/chainimpl.h>
  #include <rtems/score/bucket.h>
  #include <rtems/score/watchdog.h>
  #include <rtems/score/watchdogimpl.h>
  
  /* Doubly Linked List implementation */
  #include<stdio.h>
  #include<stdlib.h>

  struct Element  {
	Watchdog_Control* data;
	struct Element* next;
	struct Element* prev;
  } typedef Element_struct;

  struct Element* head[SIZE]; // global variable - pointer to head node.

  //Creates a new Element and returns pointer to it. 
  RTEMS_INLINE_ROUTINE struct Element* GetNewElement(Watchdog_Control *x) {
        printk("1.112 ");	
      struct Element newElement;
        printk("1.113 ");
	newElement.data = x;
        printk("1.114 ");
	newElement.prev = NULL;
        printk("1.115 ");
	newElement.next = NULL;
        printk("1.116");
	return &newElement;
  }

  //Inserts a Node at head of doubly linked list
  RTEMS_INLINE_ROUTINE struct Element* InsertAtHead(Watchdog_Control* x, int bucket) {
      printk(" \n 1 \n");
      printk("%x \n",head[bucket]);
      /*if(is_init[0]==0)
      {
          int i=0;
          is_init[0]=1;
       for(i=0;i<SIZE;i++)
       {
        head[i]=NULL;
       }
      } */
      struct Element* newElement = GetNewElement(x);
      printk("1.12 \n");
	if(head[bucket] == NULL) {
	    printk("2 \n");	
             head[bucket] = newElement;
             printk("3 \n");
             printk("%x \n",head[bucket]);
		return newElement;
	}
        printk("1.13");
	head[bucket]->prev = newElement;
	newElement->next = head[bucket]; 
	head[bucket] = newElement;
        return newElement;
  }

 RTEMS_INLINE_ROUTINE  Watchdog_Control* RemoveHead(int bucket){
	Watchdog_Control* top = NULL;
	if(head[bucket]!= NULL){
		struct Element* first = head[bucket];
		top = first->data;
		head[bucket] = first->next;
		if(first->next != NULL)
		{
			first->next->prev = NULL;
			first->next = NULL;
		}

	}
	return top;
  }
  
 RTEMS_INLINE_ROUTINE  void RemoveElement (struct Element* x, int bucket){


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
  }

RTEMS_INLINE_ROUTINE void _Bucket_Initialize()
{
 int i=0;
 for( i=0; i<SIZE;i++)
 {
    head[i]=NULL;
 }
}

RTEMS_INLINE_ROUTINE bool _bucket_is_empty(int bucket)
{
    if(head[bucket]==NULL)
    {
     return true;
    }
    printk("bucket not empty!!! \n");
    return false;
}
RTEMS_INLINE_ROUTINE int Get_Bucket_Size()
{
    int i=SIZE;
 return i;
}
#endif
    /* end of include file*/ 
