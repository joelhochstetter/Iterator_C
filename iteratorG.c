/* iteratorG.c
   Generic Iterator implementation, using doubly linked list

   Written by: Joel Hochstetter
   Student no: z5115275
   Date: 20/04/2018



*/



#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "iteratorG.h"
    
#define FORWARD 0
#define BACKWARD 1


typedef struct Node * NodeP;

typedef struct Node 
{
    void *val;
    NodeP nodePrev;
    NodeP nodeNext;
} Node;

typedef struct IteratorGRep 
{
    NodeP head; //points to first node in the iterator
    NodeP tail; //points to last node in the iterator
    NodeP itPrev; //points to element to left of cursor
    NodeP itNext; //points to element to right of cursor
    int direction; //it is run in either FORWARD or BACKWARD direction
    int disFromStart; //# of nodes between head and cursor inclusively
    int disFromEnd; //# of nodes between tail and cursor inclusively
    ElmCompareFp cmp;
    ElmFreeFp free;
    ElmNewFp copy;
    
} IteratorGRep;

IteratorG newIterator(ElmCompareFp cmp, ElmNewFp copy, ElmFreeFp free) 
{
//O(1). Creates new IteratorG
    IteratorG newIt = malloc(sizeof(IteratorGRep));
	 if (newIt  == NULL) { //if malloc fails
	  fprintf(stderr, "Error!!  Failed to allocate space for newIt\n");
	  return NULL;
	}
    newIt->head = NULL;
    newIt->tail = NULL;
    newIt->itPrev = NULL;
    newIt->itNext = NULL;
    newIt->direction = FORWARD; //default direction forward
    newIt->disFromStart = 0;
    newIt->disFromEnd = 0;
    newIt->cmp = cmp;
    newIt->copy = copy;
    newIt->free = free;
    return newIt;
}

int add(IteratorG it, void *vp) 
{
//O(1). Adds element immediately before current cursor position
    int result = 0;
    NodeP newNode =  malloc(sizeof(Node));
    if (newNode != NULL) { //if malloc succeeds
        newNode->val = it->copy(vp);
        if (it->disFromStart > 0) { //links itPrev to newNode
            it->itPrev->nodeNext = newNode;            
        } else { //distance from start = 0
            it->head = newNode;
        }
        if (it->disFromEnd > 0) { //links itNext to newNode
            it->itNext->nodePrev = newNode;            
        } else { //distance from end = 0
            it->tail = newNode;
        }
        newNode->nodeNext = it->itNext;
        newNode->nodePrev = it->itPrev;
        //adjust cursor position depending on direction of it
        if (it->direction == FORWARD) { 
            it->itNext = newNode;
            it->disFromEnd ++;
        } else { //list is in backward direction
            it->itPrev = newNode; 
            it->disFromStart ++; 
        }
        result = 1;
    } else {
        result = 0;
    }
    return result;    
}


int hasNext(IteratorG it)
{
    //O(1). returns 1 if next element exists and 0 if not
    //note itPrev is the next element if running BACKWARDs
    int result = 0;
    if (((it->direction == FORWARD) && (it->disFromEnd > 0)) 
    || ((it->direction == BACKWARD) && (it->disFromStart > 0))) {
        result = 1;
    }
    return result;
}

int hasPrevious(IteratorG it) 
{
    //O(1). returns 1 if prev element exists and 0 if not
    //note itPrev is the next element if running BACKWARDs
    int result = 0;
    if (((it->direction == FORWARD) && (it->disFromStart > 0)) 
    || ((it->direction == BACKWARD) && (it->disFromEnd > 0))) {
        result = 1;
    }
    return result;
}

void *next(IteratorG it)
{
    //O(1). returns (void *) to next element and moves cursor to right
    void * nval = NULL;
    if (it->direction == FORWARD) {
        if (it->disFromEnd > 0) {
            nval = it->itNext->val;
            it->itPrev = it->itNext;
            it->itNext = it->itNext->nodeNext;
            it->disFromStart ++;
            it->disFromEnd --;           
        }
    } else { //list is running in backward direction
        if (it->disFromStart > 0) {
            nval = it->itPrev->val;
            it->itNext = it->itPrev;
            it->itPrev = it->itPrev->nodePrev;
            it->disFromStart --;
            it->disFromEnd ++; 
        }
    }
    return nval;
}


void *previous(IteratorG it)
{
    //O(1). returns (void *) to prev element and moves cursor to left
    reverse(it);
    void * pval = next(it);
    reverse(it);
    return pval;
}

int del(IteratorG it)
{
    int result = 0;
    NodeP toDelete;
    if (it->direction == FORWARD) {
        if (it->disFromStart > 0) {
            toDelete = it->itPrev;
            it->itPrev = toDelete->nodePrev;
            if (it->disFromStart > 1) {
                toDelete->nodePrev->nodeNext = toDelete->nodeNext;
            } else {
                it->head = toDelete->nodeNext;
            }
            if (it->disFromEnd >= 1) {
                toDelete->nodeNext->nodePrev = toDelete->nodePrev;
            } else {
                it->tail = toDelete->nodePrev;
            }
            it->free(toDelete->val);
            free(toDelete);
            result = 1;
            it->disFromStart --;
        }    
    } else {
        if (it->disFromEnd > 0) {
            toDelete = it->itNext;
            it->itNext = toDelete->nodeNext;
            if (it->disFromEnd >= 1) {
                toDelete->nodeNext->nodePrev = toDelete->nodePrev;
            } else {
                it->tail = toDelete->nodePrev;            
            }
            if (it->disFromStart > 1) {
                toDelete->nodePrev->nodeNext = toDelete->nodePrev;
            } else {
                it->head = toDelete->nodeNext;
            }
            it->free(toDelete->val);
            free(toDelete);
            result = 1;  
            it->disFromEnd --;  
        }   
    }
    return result;  
}

int set(IteratorG it, void *vp)
{
    int result = 0;
    if (it->direction == FORWARD) {
        if (it->disFromStart >= 1) {
            it->free(it->itPrev->val);
            it->itPrev->val = it->copy(vp);
            result = 1;
        }
    } else { //direction is backward
        if (it->disFromEnd >= 1) {
            it->free(it->itNext->val);
            it->itNext->val = it->copy(vp);
            result = 1;            
        }        
    }
    return result;
}

IteratorG advance(IteratorG it, int n)
{   
    //[  82 ^ 6 12 25] 
    //n < 0 backward => abs(n) < disfromend
    //n > 0 backward => abs(n) < disfromstart
    //n > 0 forward => abs(n) < disfromend
    //n < 0 forward => abs(n) < disfromstart



    IteratorG visitIt = NULL;
    int reversedList = 0;
    if (n < 0) {
        reversedList = 1;
        reverse(it);
    }
    if ((abs(n) <= distanceToEnd(it)) && (n != 0)) {      
        visitIt = newIterator(it->cmp, it->copy, it->free);
        assert(visitIt != NULL);
        int i;
        for (i = 0 ; i < abs(n) ; i++) {
            add(visitIt,it->copy(next(it))); //adds to start of visitIt and moves to next element
        }
        reverse(visitIt); //since was created out of order
        reset(visitIt);  //points head at start
    }
    if (reversedList == 1) {
        reverse(it);
    }
    return visitIt;
}

void reverse(IteratorG it)
{
//makes direction FORWARD (0) if BACKWARD (1) and vice versa
    it->direction = (it->direction + 1) % 2;
}

IteratorG find(IteratorG it, int (*fp) (void *vp))
{
    NodeP currPrev = it->itPrev;
    NodeP currNext = it->itNext;
    IteratorG foundIt = newIterator(it->cmp, it->copy, it->free);
    int reversedList = 0;
    int i;
    while (hasNext(it) == 1) {
        if (fp(curr->val) == 1)
            add(foundIt,it->copy(next(it))); //adds to start of visitIt and moves to next element
    }
    reverse(foundIt); //since was created out of order
    reset(foundIt);  //points head at start
    it->itNext = currNext;
    it->itPrev = currPrev;
    return foundIt;
}

int distanceFromStart(IteratorG it)
{
    int result;
    if (it->direction == FORWARD) {
        result = it->disFromStart;
    } else { //list is running in backward direction
        result = it->disFromEnd;        
    }    
    return result;
}

int distanceToEnd(IteratorG it)
{
    int result;
    if (it->direction == FORWARD) {
        result = it->disFromEnd;
    } else { //list is running in backward direction
        result = it->disFromStart;        
    }
    return result;
}

void reset(IteratorG it)
{
    if (it->direction == FORWARD) {
        it->itNext = it->head;
        it->itPrev = NULL;
        it->disFromEnd += it->disFromStart;
        it->disFromStart = 0;
    } else { //direction is backwards
        it->itPrev = it->tail;
        it->itNext = NULL;  
        it->disFromStart += it->disFromEnd;
        it->disFromEnd = 0;      
    }
}

void freeIt(IteratorG it)
{
    assert(it != NULL);
    NodeP toDelete;
    it->itNext = it->head;
    while (it->itNext != NULL) {
        toDelete = it->itNext;
        it->itNext = it->itNext->nodeNext;
        it->free(toDelete->val);
        free(toDelete);
    }
    free(it);
}
