#include "mem.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

// Global Variables *****************
#define FIRST_FIT 100
#define BEST_FIT 111
#define WORST_FIT 112

struct mem_node *freeHead = NULL;
int freeSize = 0;
int m_error;


// defining the protypes *****************
struct mem_node* consolidateBefore(struct mem_node *newNode);
struct mem_node* consolidateAfter(struct mem_node *newNode);

// Declaring his struct type *****************
struct mem_node {
	struct mem_node *next;
	struct mem_node *prev;
	int	size;
	int policy; // ************************ ADDED THIS TO THE CODE ************************
};

// I THINK YOU SHOULD DELETE THIS MAIN METHOD SINCE IT ISNT BEENING USED BY AYTHING. 
int main() {
	return 0;
}

// this functin is only called once and 
int Mem_Init(int sizeOfRegion, int debug)
{
	if(sizeOfRegion > 0 && freeHead == NULL )
	{
		int pageSize = getpagesize();
		if(sizeOfRegion % getpagesize() !=f 0)
			sizeOfRegion += getpagesize() - (sizeOfRegion % getpagesize());
		

		int fd = open("/dev/zero", O_RDWR);

		void* ptr = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
		// MAP_FAILED is a constant defined by a library other than our *****************
		if (ptr == MAP_FAILED) { perror("mmap"); exit(1); }
		close(fd);

		//get 8 byte alligned
		// uintptr_t is an unsigned integer data type that accepts pointers 
		// so its casting the ptr to see if its divisable by 8 mod
		if((uintptr_t)ptr  % 8 != 0)
		{
			// Seems like he is rounding the pointer up
			ptr += 8 - ((uintptr_t)ptr % 8); 
			// Seems like he is making the request region smaller so you wont have to 
			// use all that size that is passed originally
			sizeOfRegion -= 8 - ((uintptr_t)ptr % 8); 
		}
		// Because you finally called Mem_Init the Global freeHead should now point to 
		// the begininng of the pointer ptr
		// Created the first block but it dosent point to anything
		freeHead = ptr;
		freeHead->next = NULL;
		freeHead->prev = NULL;
		freeHead->size = sizeOfRegion;
		freeHead->policy = policy;

		freeSize = sizeOfRegion; // CAN YOU SEE WHERE ELSE THEY USE THIS freeSize variable becasue i dont see how he is using it in any of his code besides here and the global variable. 
		return 0;
	}else {
		m_error = E_BAD_ARGS;
		return -1;
	}
}

void *Mem_Alloc(int size)
{
	struct mem_node *biggestNode  = freeHead;
	struct mem_node *currentNode  = freeHead;

	if(size % 8 != 0)
		size += 8 - (size % 8);

	if(freeHead == NULL)
	{
		//our list is empty
		m_error = E_NO_SPACE;
		return NULL;
	}

// ************************ ADDED THIS TO THE CODE ************************
	if(freeHead->policy == FIRST_FIT){
		// EXECUTE CODE FOR FIRST FIT
	} else if (freeHead->policy == BEST_FIT){
		// EXECUTE CODE FOR BEST FIT
	} else {
		// EXECUTE CODE FOR WORST FIT
	}

	//find the biggest free chunk (worst fit)
	do{
		if(currentNode->size > biggestNode->size)
			biggestNode = currentNode;

		currentNode = currentNode->next;
	}while(currentNode != NULL);

	struct mem_node* biggestNodePrev = biggestNode->prev;
	struct mem_node* biggestNodeNext = biggestNode->next;
	int biggestNodeSize = biggestNode->size;

	//check for freespace
	if(biggestNode->size < size + 24)
	{
		m_error = E_NO_SPACE;
		return NULL;
	}else if(biggestNode->size == size + 24) {
		if(biggestNode != freeHead)
		{
			biggestNode->prev->next = biggestNodeNext;
		}
		if(biggestNodeNext != NULL)
		{
			biggestNode->next->prev = biggestNodePrev;
		}
		if(biggestNode == freeHead)
		{
			freeHead = biggestNodeNext;
		}

		uintptr_t *new_size = (uintptr_t*)biggestNode;
		*new_size = size;
		return (void*)new_size + 24;
	}else {
		void *returnVal;
		uintptr_t *new_size = (uintptr_t*)biggestNode;
		returnVal =  (void*)new_size + 24;

		struct mem_node* newNode = returnVal + size;
		
		newNode->prev = biggestNodePrev;
		newNode->next = biggestNodeNext;
		newNode->size = biggestNodeSize - (size + 24);

		if(biggestNode != freeHead)
		{
			biggestNode->prev->next = newNode;
		}
		if(biggestNode->next != NULL)
		{
			biggestNode->next->prev = newNode;
		}
		if(biggestNode == freeHead)
		{
			freeHead = newNode;
		}

		*new_size = size;
		return returnVal;
	}


}

int Mem_Free(void *ptr)
{
	if(ptr == NULL)
		return 0;
	
	int *size = ptr - 24;

	struct mem_node *beforeNode = freeHead;
	struct mem_node *newNode = (struct mem_node*)size;

	if(beforeNode == NULL)
	{
		//Freed chunk size of our entire init size
		
		newNode->size = *size + 24;
		newNode->prev = NULL;
		newNode->next = NULL;
		freeHead = newNode;
		return 0;
	}else if((uintptr_t*)size < (uintptr_t*)freeHead)
	{
		//Freed memory at beginning of list

		newNode->size = *size + 24;
		newNode->prev = NULL;
		newNode->next = freeHead;
		freeHead->prev = newNode;
		freeHead = newNode;
	}else {

		while((void*)beforeNode->next < (void*)size && beforeNode->next != NULL)
		{
			beforeNode = beforeNode->next;
		}

		if (beforeNode == freeHead)
		{
			newNode->size = *size + 24;
			newNode->prev = beforeNode;
			newNode->next = beforeNode->next;
			newNode->next->prev = newNode;
			beforeNode->next = newNode;
		}
		else if(beforeNode->next == NULL)
		{
			newNode->size = *size + 24;
			newNode->prev = beforeNode;
			newNode->prev->next = newNode;
			newNode->next = beforeNode->next;
			beforeNode->next = newNode;
		}
		else if(beforeNode != freeHead)
		{
			newNode->size = *size + 24;
			newNode->prev = beforeNode;
			newNode->next = beforeNode->next;
			beforeNode->next = newNode;
			newNode->next->prev = newNode;
		}
	}

	consolidateAfter(consolidateBefore(newNode));
	return 0;
}

struct mem_node* consolidateBefore(struct mem_node *newNode)
{
	struct mem_node *prevNode = newNode->prev;
	if(prevNode != NULL)
	{
		if(((void*)prevNode + prevNode->size) == newNode)
		{
			prevNode->next = newNode->next;
			prevNode->size += newNode->size;
			prevNode->next->prev = prevNode;
			return prevNode;
		}
	}
	return newNode;
}

struct mem_node* consolidateAfter(struct mem_node *newNode)
{
	struct mem_node *nextNode = newNode->next;
	if(nextNode != NULL)
	{
		if(((void*)newNode + newNode->size) == nextNode)
		{
			newNode->next = nextNode->next;
			if(newNode->next != NULL)
				newNode->next->prev = newNode;

			newNode->size += nextNode->size;
		}
	}
	return newNode;
}

void Mem_Dump(){
	struct mem_node *nextNode = freeHead;
	while(nextNode != NULL)
	{
		printf("Node at: %p\n\tPrevious: %p\n\tNext: %p\n\tSize: %d\n", nextNode, nextNode->prev, nextNode->next, nextNode->size);
		nextNode = nextNode->next;
	}
}