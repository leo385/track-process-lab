#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>


//Global memory array for threads
static char* buffr_mem = NULL;

//Wątki
void Reader(const char* file)
{
	 if(!fopen(file, "r"))
	 {
		  printf("Cannot read the file");
	 }
}


int main(void)
{
	char user_name[10] = "\0";

	int *number = malloc(sizeof *number);

	printf("Something...\n");
	printf("Give your name: ");
	
	fgets(user_name, sizeof(user_name), stdin);
	printf("Hello %s", user_name);

	assert(2+2==4);

	free(number);

	return EXIT_SUCCESS;

}
