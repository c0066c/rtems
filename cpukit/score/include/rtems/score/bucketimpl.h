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
  
  /* Doubly Linked List implementation */
  #include<stdio.h>
  #include<stdlib.h>

  struct Element  {
	routine* data;
	struct Element* next;
	struct Element* prev;
  };

  struct Element* head[30]; // global variable - pointer to head node.

  //Creates a new Element and returns pointer to it. 
  struct Element* GetNewElement(routine* x) {
	struct Element* newElement
		= (struct Element*)malloc(sizeof(struct Element));
	newElement->data = x;
	newElement->prev = NULL;
	newElement->next = NULL;
	return newElement;
  }

  //Inserts a Node at head of doubly linked list
  void InsertAtHead(routine* x, int bucket) {
	struct Element* newElement = GetNewElement(x);
	if(head[bucket] == NULL) {
		head[bucket] = newElement;
		return;
	}
	head->prev = newElement;
	newNode->next = head[bucket]; 
	head[bucket] = newElement;
  }

  routine RemoveHead(int bucket){
	routine* top = NULL;
	if(head[bucket]!= NULL){
		Node first = &head[bucket];
		routine* top = first -> routine;
		head[bucket] = first -> next;
		if(first -> != NULL)
		{
			first.next -> prev = NULL;
			first -> next = NULL;
		}

	}
	return top;
  }
  
  routine RemoveElement (Element x, int bucket){


	if(x==&(head[bucket]))
	{
		head[bucket] = x -> next;
	}
	if(x->next != NULL && x-> prevents!= NULL)
	{
		x.prev -> next = x -> next;
		x.next -> prev = x -> prev;
	}
	else if(x->prev == NULL && x -> next != NULL)
	{
		x.next -> prev == NULL;
	}
	else if(if(x -> prev != NULL && x -> next == NULL)
	{
		x.prev -> next == NULL; 
	}
  }

  #endif
    /* end of include file*/ 
