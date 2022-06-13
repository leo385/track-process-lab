#ifndef CPU_H
#define CPU_H

#include <stdlib.h> // some macros ex. EXIT_SUCCESS.
#include <stdio.h> // i/o stream
#include <assert.h> //for assertion instead of using unit test scripts.
#include <unistd.h> //for sleep().
#include <pthread.h> //for threads
#include <semaphore.h> // for sem.
#include <string.h> //string features

//macro for buffer to get lines with specific length
#define MAX_LINES_L 100
#define MAX_LENGTH_L 1000

#define CORE_NUM 4
#define THREAD_NUM 10
#define BUFF_SIZE 10

//Using mutex to conclusion between threads.
extern pthread_mutex_t mutex;

//semaphores to signal either is empty or full, for producent and consumer.
extern sem_t empty;
extern sem_t full;



typedef unsigned long long int ull;


typedef struct Cpu 
{
	ull _usertime;
	ull _nicetime;
	ull _systemtime;
	ull _idletime;
	ull _io_wait;
	ull _irq;
	ull _soft_irq;
	ull _steal;
	ull _guest;
	ull _guest_nice;

	struct Cpu *me;

}Cpu;


void Core_ctor(struct Cpu * const me, ull usertime, ull nicetime, ull systemtime, ull idletime, ull io_wait, ull irq, ull soft_irq);


/*I've used this split function, because strtok(); works bad with threads and affect on performance too.*/
int split (const char *txt, char delim, char ***tokens);

/* freeing tokens from split function, tokens accumulate char* from lines where has encountered ' ' and separate words in dynamic alloc/dea array(tokens). */
void freeingToken();


/* In this function I wanted convert founded strings by tokens to (unsigned long long int) with function atoll() and strtoll(),
 * but i've got SegFault and I could not go through this problem.
 * I tried search an alternative function to this and didn't find unfortunately.
 *
 * */
void initCPU(unsigned int cpu, int line);


//Function that read from file and perform iniCPU().
void* Reader(void* file);


//Performing sended values from Reader and calculate it in arrays for all cores.
void* Analyzer();


//Print results in specific format
void* Printer();

#endif
