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


typedef unsigned long long int ull;

//semaphores to signal either is empty or full, for producent and consumer.
sem_t empty;
sem_t full;

//for increment/dec in buffers during produce/consume process.
int in = 0;
int out = 0;

//for tokens - it works with split function
char **tokens = NULL;
int count = 0;

//Global memory array for strings in /proc/stat
static char buffr_mem[MAX_LINES_L][MAX_LENGTH_L];

//Using mutex to better maintenance.
pthread_mutex_t mutex;


FILE* dataFile = NULL;

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


void initCPU(size_t line)
{	
		count = split(&buffr_mem[line][1000], ' ', &tokens);
		Core_ctor(&core[line], atoll(tokens[1]), atoll(tokens[2]), atoll(tokens[3]), atoll(tokens[4]), atoll(tokens[5]), atoll(tokens[6]), atoll(tokens[7]));
		
		freeingToken();
}


void* Reader(void* file)
{

	 dataFile = fopen(file, "r");

	 if(dataFile == NULL)
	 {
		  printf("Cannot read the file.\n");
	 }

	int line = 0;

	while(!feof(dataFile) && !ferror(dataFile))
	{
			if(fgets(buffr_mem[line], MAX_LENGTH_L, dataFile) != NULL)
			{
				line++;
			}
	}
			
	fclose(dataFile);


	//initialize and assignment cpu for consumer.
	for(int i = 0; i < 4; i++)
	{
		sem_wait(&empty);
		pthread_mutex_lock(&mutex);

		core = calloc(4, sizeof(*core));

		initCPU(i);

		//freeing cores
		free(core);

		pthread_mutex_unlock(&mutex);
		sem_post(&full);
	}

	 return NULL;
}


void* Analyzer()
{
	for(int i = 0; i < 4; i++)
	{
		sem_wait(&full);
		pthread_mutex_lock(&mutex);


		pthread_mutex_unlock(&mutex);
		sem_post(&empty);
	}

	return NULL;
}

int main(void)
{
	pthread_t cpu_id[4];
	pthread_mutex_init(&mutex, NULL);
	sem_init(&empty, 0, 4);
	sem_init(&full, 0, 0);	

	pthread_create(&cpu_id[0], NULL, Reader, "/proc/stat");

	pthread_join(cpu_id[0], NULL);
	
	

	return EXIT_SUCCESS;

}
