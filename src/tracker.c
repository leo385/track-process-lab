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
#define THREAD_NUM 7
#define BUFF_SIZE 7


typedef unsigned long long int ull;


//for tokens - it works with split function
char **tokens = NULL;
int count = 0;

//Global memory array for strings in /proc/stat
static char buffr_mem[MAX_LINES_L][MAX_LENGTH_L];

//Array to send values by producer and consume by consumer.
ull buffer[CORE_NUM][BUFF_SIZE];

//for increment/dec in buffers during produce/consume process.
int in = 0;
int out = 0;

//Using mutex to conclusion between threads.
pthread_mutex_t mutex;

//semaphores to signal either is empty or full, for producent and consumer.
sem_t empty;
sem_t full;

FILE* dataFile = NULL;

double CPU_PERCENTAGE[CORE_NUM];

#ifndef CPU_H
#define CPU_H

struct Cpu;

typedef struct
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

void Core_ctor(Cpu * const me, ull usertime, ull nicetime, ull systemtime, ull idletime,
				ull io_wait, ull irq, ull soft_irq)
{
	me->_usertime = usertime;
	me->_nicetime = nicetime;
	me->_systemtime = systemtime;
	me->_idletime = idletime;
	me->_io_wait = io_wait;
	me->_irq = irq;
	me->_soft_irq = soft_irq;
}

#endif /* CPU_H */

/*I've used this split function, because strtok(); works bad with threads and affect on performance too.*/
int split (const char *txt, char delim, char ***tokens)
{
    int *tklen, *t, count = 1;
    char **arr, *p = (char *) txt;

    while (*p != '\0') if (*p++ == delim) count += 1;
    t = tklen = calloc (count, sizeof (int));
    for (p = (char *) txt; *p != '\0'; p++) *p == delim ? *t++ : (*t)++;
    *tokens = arr = malloc (count * sizeof (char *));
    t = tklen;
    p = *arr++ = calloc (*(t++) + 1, sizeof (char *));
    while (*txt != '\0')
    {
        if (*txt == delim)
        {
            p = *arr++ = calloc (*(t++) + 1, sizeof (char *));
            txt++;
        }
        else *p++ = *txt++;
    }
    free (tklen);
    return count;
}


static Cpu* core = NULL;

void freeingToken()
{	
	//freeing tokens
	for(int i = 0; i < count; i++)
	{
		free(tokens[i]);
	}
	
	free(tokens);
}


void initCPU(unsigned int cpu, size_t line)
{
		//split columns from one line by ' ' char and get it to **tokens.
		count = split(&buffr_mem[line][1000], ' ', &tokens);
		Core_ctor(&core[line], atoll(tokens[1]), atoll(tokens[2]), atoll(tokens[3]), atoll(tokens[4]), atoll(tokens[5]), atoll(tokens[6]), atoll(tokens[7]));	

		//assignment values from tokens to buffer with converting to unsigned ll int.
		buffer[cpu][in] = atoll(tokens[in + 1]);

}


void* Reader(void* file)
{
	for(;;)
	{

			int line = 0;
			
			dataFile = fopen(file, "r");

			 if(dataFile == NULL)
			 {
				  printf("Cannot read the file.\n");
			 }


			//count lines in file and get to buffer
			while(!feof(dataFile) && !ferror(dataFile))
			{
					if(fgets(buffr_mem[line], MAX_LENGTH_L, dataFile) != NULL)
					{
						line++;
					}
			}
		

			sleep(1);

		   //Add to the buffer		 
			sem_wait(&empty);
			pthread_mutex_lock(&mutex);

			//assign values from **tokens to buffer array.
			for(int i = 0; i < 4; i++)
			{
				initCPU(i, i);	
			}

			//incrementing in buffer from initCPU().
			in = (in+1)%BUFF_SIZE;
	
			//freeing tokens to update values there.
			freeingToken();

			pthread_mutex_unlock(&mutex);
			sem_post(&full);
	}
}


void* Analyzer()
{

	//initialize help variables
	ull PrevIdle[CORE_NUM][MAX_LENGTH_L], PrevIOWait[CORE_NUM][MAX_LENGTH_L], Idle[CORE_NUM][MAX_LENGTH_L], IOWait[CORE_NUM][MAX_LENGTH_L],
	PrevNonIdle[CORE_NUM][MAX_LENGTH_L], PrevUser[CORE_NUM][MAX_LENGTH_L], PrevNice[CORE_NUM][MAX_LENGTH_L], PrevSystem[CORE_NUM][MAX_LENGTH_L],
	PrevIrq[CORE_NUM][MAX_LENGTH_L], PrevSoftIrq[CORE_NUM][MAX_LENGTH_L], PrevSteal[CORE_NUM][MAX_LENGTH_L], NonIdle[CORE_NUM][MAX_LENGTH_L],
	PrevTotal[CORE_NUM][MAX_LENGTH_L], Total[CORE_NUM][MAX_LENGTH_L], totald[CORE_NUM][MAX_LENGTH_L], idled[CORE_NUM][MAX_LENGTH_L];

	for(int j=0; j < 1000; j++)
	{

		//Remove from the buffer
		sem_wait(&full);
		pthread_mutex_lock(&mutex);
	
		//Retrieve previous values from buffer to cores
		for(int i = 0; i < 4; i++)
		{
			Idle[i][j] = buffer[i][3];
			IOWait[i][j] = buffer[i][4];

			PrevUser[i][j] = buffer[i][0];
			PrevNice[i][j] = buffer[i][1];
			PrevSystem[i][j] = buffer[i][2];
			PrevIdle[i][j] = buffer[i][3];
			PrevIOWait[i][j] = buffer[i][4];
			PrevIrq[i][j] = buffer[i][5];
			PrevSoftIrq[i][j] = buffer[i][6];
			PrevSteal[i][j] = 0;

			core[i]._usertime = PrevUser[i][j];
			core[i]._nicetime = PrevNice[i][j];
			core[i]._systemtime = PrevSystem[i][j];
			core[i]._idletime = PrevIdle[i][j];
			core[i]._io_wait = PrevIOWait[i][j];
			core[i]._irq = PrevIrq[i][j];
			core[i]._soft_irq = PrevSoftIrq[i][j];
			core[i]._steal = PrevSteal[i][j];
			core[i]._guest = 0;
			core[i]._guest_nice = 0;


			//Calculating usage of cpu
			PrevIdle[i][j-1] = PrevIdle[i][j-1] + PrevIOWait[i][j-1];
			Idle[i][j] = Idle[i][j] + IOWait[i][j];
			
			PrevNonIdle[i][j] = PrevUser[i][j-1] + PrevNice[i][j-1] + PrevSystem[i][j-1] + PrevIrq[i][j-1] + PrevSoftIrq[i][j-1] + PrevSteal[i][j-1];
			NonIdle[i][j] = core[i]._usertime + core[i]._nicetime + core[i]._systemtime + core[i]._irq + core[i]._soft_irq + core[i]._steal; 
		
			PrevTotal[i][j] = PrevIdle[i][j-1] + PrevNonIdle[i][j];
			Total[i][j] = Idle[i][j] + NonIdle[i][j];

			totald[i][j] = Total[i][j] - PrevTotal[i][j];
			idled[i][j] = Idle[i][j] - PrevIdle[i][j-1];

			CPU_PERCENTAGE[i] = (double)(totald[i][j] - idled[i][j]) / totald[i][j];

		}
	
		
		pthread_mutex_unlock(&mutex);
		sem_post(&empty);

		/* //Consume */
		/* printf("%.1f%%\n", CPU_PERCENTAGE[1] * 100); */
		/* sleep(1); */
	}

	return NULL;
}

void* Printer(void* CPU_RESULT)
{

	return NULL;
}


int main(void)
{
	pthread_t pro[THREAD_NUM], con[THREAD_NUM];
	pthread_mutex_init(&mutex, NULL);
	sem_init(&empty, 0, THREAD_NUM);
	sem_init(&full, 0, 0);

	core = calloc(4, sizeof(*core));

	for(int i = 0; i < THREAD_NUM; i++)
	{
		//producent
		if(pthread_create(&pro[i], NULL, Reader, "/proc/stat") != 0)
			perror("Failed to create thread.\n");

		//consumer
		if(pthread_create(&con[i], NULL, Analyzer, NULL) != 0)
			perror("Failed to create thread.\n");
	}


	for(int i = 0; i < THREAD_NUM; i++)
	{
		
		if(pthread_join(pro[i], NULL) != 0)
			perror("Failed to join thread.\n");	

		if(pthread_join(con[i], NULL) != 0)
			perror("Failed to join thread.\n");

	}



	//freeing cores
	free(core);

	//destroy semaphores and mutual conclusion too.
	sem_destroy(&empty);
	sem_destroy(&full);

	pthread_mutex_destroy(&mutex);

	//destroy file
	fclose(dataFile);

	return EXIT_SUCCESS;

}
