#include "cpu.h"


double CPU_PERCENTAGE[CORE_NUM];

//for increment/dec in buffers during produce/consume process.
int in;
int out;


//Array to send values by producer and consume by consumer.
ull buffer[CORE_NUM][BUFF_SIZE];


//for tokens - it works with split function
char **tokens;
int count;


//Global memory array for strings in /proc/stat
char buffr_mem[MAX_LINES_L][MAX_LENGTH_L];


//for files handling
FILE* dataFile;


extern struct Cpu* core;


void Core_ctor(struct Cpu * const me, ull usertime, ull nicetime, ull systemtime, ull idletime,
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


void freeingToken()
{	
	//freeing tokens
	for(int i = 0; i < 7; i++)
	{
		free(tokens[i]);
	}
	
	free(tokens);
}


void initCPU(unsigned int cpu, int line)
{
		//split columns from one line by ' ' char and get it to **tokens.
		count = split(&buffr_mem[line][MAX_LENGTH_L], ' ', &tokens);
		Core_ctor(&core[line], atoll(tokens[1]), atoll(tokens[2]), atoll(tokens[3]), atoll(tokens[4]), atoll(tokens[5]), atoll(tokens[6]), atoll(tokens[7]));	

		//assignment values from tokens to buffer with converting to unsigned ll int.
		buffer[cpu][in] = atoll(tokens[in + 1]);

		//freeing tokens to update values there.
		freeingToken();

}


void* Reader(void* file)
{
	for(;;)
	{

			sleep(1);

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

			
			pthread_mutex_unlock(&mutex);
			sem_post(&full);


	}

	fclose(dataFile);
}


void* Analyzer()
{

	//initialize help variables
	ull PrevIdle[CORE_NUM][MAX_LENGTH_L], PrevIOWait[CORE_NUM][MAX_LENGTH_L], Idle[CORE_NUM][MAX_LENGTH_L], IOWait[CORE_NUM][MAX_LENGTH_L],
	PrevNonIdle[CORE_NUM][MAX_LENGTH_L], PrevUser[CORE_NUM][MAX_LENGTH_L], PrevNice[CORE_NUM][MAX_LENGTH_L], PrevSystem[CORE_NUM][MAX_LENGTH_L],
	PrevIrq[CORE_NUM][MAX_LENGTH_L], PrevSoftIrq[CORE_NUM][MAX_LENGTH_L], PrevSteal[CORE_NUM][MAX_LENGTH_L], NonIdle[CORE_NUM][MAX_LENGTH_L],
	PrevTotal[CORE_NUM][MAX_LENGTH_L], Total[CORE_NUM][MAX_LENGTH_L], totald[CORE_NUM][MAX_LENGTH_L], idled[CORE_NUM][MAX_LENGTH_L];

for(;;)
{

	for(int j=0; j < MAX_LENGTH_L; j++)
	{

		sleep(1);

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
	
	}
}
			
	return NULL;
}


void* Printer()
{
	for(;;)
	{
		fputs("\033[A\033[2K\033[A\033[2K",stdout);
		printf("CPU1: %.2f%%\n", (double)CPU_PERCENTAGE[0] * 100);	
		printf("CPU2: %.2f%%\n", (double)CPU_PERCENTAGE[1] * 100);
		printf("CPU3: %.2f%%\n", (double)CPU_PERCENTAGE[2] * 100);
		printf("CPU4: %.2f%%\n", (double)CPU_PERCENTAGE[3] * 100);
		fputs("\033[A\033[2K\033[A\033[2K",stdout);
		sleep(1);
	}

	return NULL;
}

