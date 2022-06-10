#include <stdlib.h> // some macros ex. EXIT_SUCCESS.
#include <stdio.h> // i/o stream
#include <assert.h> //for assertion instead of using unit test scripts.
#include <unistd.h> //for sleep().
#include <pthread.h> //for threads
#include <semaphore.h> // for sem.
#include <string.h> //string features


#define MAX_LINES_L 100
#define MAX_LENGTH_L 1000


typedef unsigned long long int ull;

//semaphores
sem_t empty;
sem_t full;

//for increment/dec in buffer.
int in = 0;
int out = 0;

//for tokens
char **tokens = NULL;
int count = 0;

//Global memory array for strings in /proc/stat
char buffr_mem[MAX_LINES_L][MAX_LENGTH_L];

//mutex rules
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

void Core_dtor(Cpu * const _me)
{
	if(_me != NULL)
		free(_me->me);
}

#endif /* CPU_H */

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

int initToken_val(unsigned int line)
{	
 	count = split(&buffr_mem[line][1000], ' ', &tokens);
	return count;
}


void* Reader(void* file)
{
	 dataFile = fopen(file, "r");

	 if(dataFile == NULL)
	 {
		  printf("Cannot read the file.\n");
	 }
	
	 
	sleep(1);
		
	
	int line = 0;

	while(!feof(dataFile) && !ferror(dataFile))
	{
			if(fgets(buffr_mem[line], MAX_LENGTH_L, dataFile) != NULL)
			{
				line++;
			}
	}

	fclose(dataFile);

//	core = calloc(4, sizeof * core);


	char **tokens;
	int count = 0;
	

	//CPU00

	/* core[0].usertime = atoll(tokens[1]); */
	/* core[0].nicetime = atoll(tokens[2]); */
	/* core[0].systemtime = atoll(tokens[3]); */
	/* core[0].idletime = atoll(tokens[4]); */
	/* core[0].io_wait = atoll(tokens[5]); */
	/* core[0].irq = atoll(tokens[6]); */
	/* core[0].soft_irq = atoll(tokens[7]); */

	//CPU01



//	printf("%s", &buffr_mem[3][1000]);	


	//freeing tokens
	for(int i = 0; i < count; i++)
	{
		free(tokens[i]);
	}
	
	free(tokens);

	/* free(core); */
	/* core = NULL; */

	 return NULL;
}



int main(void)
{
	pthread_t id_thread;

	pthread_create(&id_thread, NULL, Reader, "/proc/stat");
	pthread_join(id_thread, NULL);
	

	return EXIT_SUCCESS;

}
