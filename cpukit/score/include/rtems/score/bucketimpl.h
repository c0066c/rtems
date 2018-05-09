  /* This file implements the  operations for the buckets, including: initilisation, insertion and removing
   * And it includes the following logical opertion: Next first
   * It contains also an array that represents the chain of buckets
   */


  #ifndef _RTEMS_SCORE_BUCKETIMPL_H
  #define _RTEMS_SCORE_BUCKETIMPL_H




  /* defines the Size of the array */
  #define SIZE 32
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

  struct Element* head[30]; // global variable - pointer to head node.

  //Creates a new Element and returns pointer to it. 
  struct Element* GetNewElement(Watchdog_Control* x) {
	struct Element* newElement
		= (struct Element*)malloc(sizeof(struct Element));
	newElement->data = x;
	newElement->prev = NULL;
	newElement->next = NULL;
	return newElement;
  }

  //Inserts a Node at head of doubly linked list
  struct Element* InsertAtHead(Watchdog_Control* x, int bucket) {
	struct Element* newElement = GetNewElement(x);
	if(head[bucket] == NULL) {
		head[bucket] = newElement;
		return newElement;
	}
	head[bucket]->prev = newElement;
	newElement->next = head[bucket]; 
	head[bucket] = newElement;
        return newElement;
  }

  Watchdog_Control* RemoveHead(int bucket){
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
  
  void RemoveElement (struct Element* x, int bucket){


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



  #endif
    /* end of include file*/ 
