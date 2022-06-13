#include "cpu.h"

#include <stdlib.h> // some macros ex. EXIT_SUCCESS.
#include <stdio.h> // i/o stream
#include <assert.h> //for assertion instead of using unit test scripts.
#include <unistd.h> //for sleep().
#include <pthread.h> //for threads
#include <semaphore.h> // for sem.
#include <string.h> //string features

//Using mutex to conclusion between threads.
pthread_mutex_t mutex;

//semaphores to signal either is empty or full, for producent and consumer.
sem_t empty;
sem_t full;


//cores
Cpu* core = NULL;

int main(void)
{
	pthread_t rea[THREAD_NUM], ana[THREAD_NUM], pri; 
	pthread_mutex_init(&mutex, NULL);
	sem_init(&empty, 0, THREAD_NUM);
	sem_init(&full, 0, 0);

	core = calloc(4, sizeof(*core));

	for(int i = 0; i < THREAD_NUM; i++)
	{
		//producent, Reader
		if(pthread_create(&rea[i], NULL, Reader, "/proc/stat") != 0)
			perror("Failed to create thread.\n");

		//consumer, Analyzer
		if(pthread_create(&ana[i], NULL, Analyzer, NULL) != 0)
			perror("Failed to create thread.\n");

	}

	
	//Printer
	if(pthread_create(&pri, NULL, Printer, NULL) != 0)
		 perror("Failed to create thread.\n");


	for(int i = 0; i < THREAD_NUM; i++)
	{
		
		if(pthread_join(rea[i], NULL) != 0)
			perror("Failed to join thread.\n");	

		if(pthread_join(ana[i], NULL) != 0)
			perror("Failed to join thread.\n");

	}

		if(pthread_join(pri, NULL) != 0)
			perror("Failed to join thread.\n");


	//freeing cores
	free(core);

	//destroy semaphores and mutual conclusion too.
	sem_destroy(&empty);
	sem_destroy(&full);

	pthread_mutex_destroy(&mutex);


	return EXIT_SUCCESS;

}
