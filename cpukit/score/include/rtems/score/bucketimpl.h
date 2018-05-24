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
  //struct Element* reference;

  //Creates a new Element and returns pointer to it. 
  RTEMS_INLINE_ROUTINE struct Element* GetNewElement(Watchdog_Control *x) {	
      struct Element newElement;
	newElement.data=NULL;
        newElement.data = x;
        printk("data: %x\n",newElement.data);
	newElement.prev = NULL;
	newElement.next = NULL;
        printk("newElement next: %x \n",newElement.next);
	return &newElement;
  }

  //Inserts a Node at head of doubly linked list
  RTEMS_INLINE_ROUTINE struct Element* InsertAtHead(Watchdog_Control* x, int bucket) {
      printk(" \n 1 \n");
      printk("%x , %d\n",head[bucket], bucket);
      struct Element* newElement = GetNewElement(x);
      printk("directy after creation: %x \n",head[bucket]->next);
      printk("Data after insert: %x",head[bucket]->data);
	if(head[bucket] == NULL) {	
             head[bucket] = newElement;
             printk("%x , %d\n",head[bucket], bucket);
             printk("here next: %x \n",head[bucket]->next);
		return newElement;
	}
        printk("danger asdasdasd 1.13!!!!!!!!!!!!!!!!!");
	head[bucket]->prev = newElement;
	newElement->next = head[bucket]; 
	head[bucket] = newElement;
        return newElement;
  }

 RTEMS_INLINE_ROUTINE  Watchdog_Control* RemoveHead(int bucket){
	printk("first next frint %x",head[bucket]->next);
        printk("remove head triggerd \n");
        Watchdog_Control* top = NULL;
        printk("Bucket pointer :%x\n",head[bucket]);
	if(head[bucket]!= NULL){
		struct Element* first = NULL;
                first = head[bucket];
                printk("first: %x\n",first);
		top = first->data;
                printk("top: %x \n", top);
                printk("first not empty: %d",bucket);
                printk("first -> next: %x \n",first->next);
                //printk("reference-> next: %x", reference->next);
		if(first->next!=NULL)
		{
                        printk("here not null");
			first->next->prev = NULL;
			first->next = NULL;
                        head[bucket]=first->next;
		}
                else
                {
                    printk("null");
                    head[bucket]=NULL;
                }

	}
        printk("head removed \n");
        if(top==NULL)
        {
            printk("top = NULL");
        }
	return top;
  }
  
 RTEMS_INLINE_ROUTINE  void RemoveElement (struct Element* x, int bucket){

        printk("remove element trigggerd \n");
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
 //reference=GetNewElement(NULL);
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
