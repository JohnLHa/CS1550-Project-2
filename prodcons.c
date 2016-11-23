/*John Ha
jlh238@pitt.edu
CS1550 Project 2*sizeof*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <sys/wait.h>

//Same struct from sys.c, but used to initialize variables here.
struct cs1550_sem {
	int value;
	struct Node *first; 
	struct Node *last; 
};

// Wrapper function
void up(struct cs1550_sem *sem) {
    syscall(__NR_cs1550_up, sem);
}

// Wrapper function
void down(struct cs1550_sem *sem) {
    syscall(__NR_cs1550_down, sem);
}


//Shared variables
int *in, *out, counter;
int pitem = 0;
int citem;
int i;


int main(int argc, char *argv[]){

	//Checks for correct input arguments from command line
	if(argc != 4){
		printf("Wrong number of arguments entered in command line.");
		exit(1);
	}
	//Saves input arguments from command line
	int numProd = atoi(argv[1]);
	int numCons = atoi(argv[2]);
	int buffSize = atoi(argv[3]);

	//Creates an allocated mapping for JUST semaphores
	void* semPtr = mmap(NULL, sizeof(struct cs1550_sem)*3, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
	
	//creating semaphores and initializing based off slides in class
	struct cs1550_sem* empty = (struct cs1550_sem *) semPtr;
	struct cs1550_sem* full = (struct cs1550_sem *) semPtr + 1*sizeof(struct cs1550_sem);
	struct cs1550_sem* mutex = (struct cs1550_sem *) semPtr + 2*sizeof(struct cs1550_sem);
	
	//Allocated memory for the shared buffer memory with integers in and out
	void* sharedBuffPtr =  mmap(NULL, sizeof(int)*(buffSize + 3), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);

	in = (int *)sharedBuffPtr;
	out = (int *)sharedBuffPtr + 1*sizeof(int);
	int *buffPtr = (int *)sharedBuffPtr + 2*sizeof(int);

	//initializing the semaphore values
	empty->first = NULL;
	full->first = NULL;
	mutex->first = NULL;
	empty->last = NULL;
	full->last = NULL;
	mutex->last = NULL;
	empty->value = buffSize;
	full->value = 0;
	mutex->value = 1;
	
	*in = 0;
	*out = 0;
	
	//Producer children processes
	for(i = 0; i<numProd; i++){
		if(fork()==0){
			while(1){
				//locks
				down(empty);
				down(mutex);
				
				//produces an item into pitem
				pitem++;
				buffPtr[*in] = pitem;
				
				//Creates an ascii value to represent Chefs
				int ascii = 65+i;
				char c = ascii;
				
				printf("Chef %c Produced: Pancake%d\n", c, pitem);
				*in = (*in+1) % buffSize;
				
				//unlocks
				up(mutex);
				up(full);
			}
		}
	}
	//Consumer children processes
	for(i =0; i<numCons; i++){
		if(fork()==0){
			while(1){
				//locks
				down(full);
				down(mutex);
				
				citem = buffPtr[*out];
				
				//Creates an ascii value to represent Customers
				int ascii = 65+i;
				char c = ascii;
		
				printf("Customer %c Consumed: Pancake%d\n", c, citem); 
				*out = (*out+1) % buffSize;
				
				//unlocks
				up(mutex);
				up(empty);
				
				//consumes item from citem
				citem--;
			}
		}
	}	
	//Delay for processes to finish.
	int waiting;
	wait(&waiting);
	return 0;
}