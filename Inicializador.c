/** @file Inicializador.c
 * 
 * @brief Modulo inicializador del productor/consumidor 
 *
 * @par       
 * COPYRIGHT NOTICE: (R) 2020 RMR. GNU licensed.
 */
/* ----------------------------------Include Spot------------------------ */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "struct.h"
#include "Memtools.h"
#include <string.h>
#include <stdlib.h>
//#include “module.h”
/*----------------------------------End of Include Spot------------------ */

/*!
 * @brief Inicializador de los productores/consumidores
 *
 * @param[in] bufferSize  Tamaño del Buffer
 * @param[in] bufferName  Nombre del Buffer
 *
 * @return int
 */
//--------------------------------Constant Spot---------------------------------------
#define STORAGE_ID "/SHM_TEST"
#define S_Prefix "s_%s"
#define DATA "Hello, World! From PID %d"
    //--------------------------------Variable Spot---------------------------------------


    //Variables de interacción con el usuario
    char *bufferName;
    int bufferSize;


//void *realloc(int *ptr, size_t size);

    char *inputString(FILE* fp, size_t size){
    //The size is extended by the input with the value of the provisional
    char *str;
    int ch;
    size_t len = 0;
    str = realloc(NULL, sizeof(char)*size);//size is start size
    if(!str)return str;
    while(EOF!=(ch=fgetc(fp)) && ch != '\n'){
        str[len++]=ch;
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=16));
            if(!str)return str;
        }
    }
    str[len++]='\0';

    return realloc(str, sizeof(char)*len);
}

typedef struct {
    int S;
    int procCount;
    int index;
    int pids[];
} Semaforo;


    //---------------------------------Function Spot--------------------------------------




int main(int argc, char * argv[])
{ 



    //Ingreso el nombre


    printf("Ingrese el nombre del buffer de memoria: ");
    bufferName = inputString(stdin, 10);
    //while((bufferName=getchar()) != '\n' && bufferName != EOF);
    //if(scanf("%c", &bufferName)!=1){
        //printf("Nombre inválido\n");
    //}
    //else{
        printf("El nombre del buffer es %s\n",bufferName);
    //}


    //Ingreso el tamaño
    printf("Ingrese el tamaño del buffer de memoria: ");
    if(scanf("%i", &bufferSize) !=1){
        printf("Número inválido\n");
    }
    else{
        printf("El tamaño del buffer será de %i\n",bufferSize);
    }

    printf("Buffer creado\n");
    typedef struct {
    int size;
    Memory data[bufferSize];
    } buffer;

    int res;
	int fd;
	int len;
	pid_t pid;
    buffer *addr;
	buffer data;
    Semaforo *sem_m;

    char *sem_msg = malloc(sizeof(char) * (strlen(bufferName) + 2));

    sprintf(sem_msg, "s_%s", bufferName);

    printf("%s\n",sem_msg);


    size_t size = sizeof(data);

	pid = getpid();
	//sprintf(data, DATA, pid);

    data.size = bufferSize;

    for(int i = 0; i < 4; i++){
        data.data[i].processID = i;
    }
	// get shared memory file descriptor (NOT a file)
	fd = shm_open(bufferName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		perror("open");
		return 10;
	}

	// extend shared memory object as by default it's initialized with size 0
	res = ftruncate(fd, size);
	if (res == -1)
	{
		perror("ftruncate");
		return 20;
	}

	// map shared memory to process address space
	addr = (buffer *) mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
	{
		perror("mmap");
		return 30;
	}

	// place data into memory
	//len = strlen(data) + 1;
	//memcpy(addr, data, len);

    addr->size = data.size;

    for(int i = 0; i < 4; i++){
        addr->data[i].processID = data.data[i].processID;
    }

    fd = shm_open(sem_msg, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

	if (fd == -1)
	{
		perror("open");
		return 10;
	}

	// extend shared memory object as by default it's initialized with size 0
	res = ftruncate(fd, size);
	if (res == -1)
	{
		perror("ftruncate");
		return 20;
	}

	// map shared memory to process address space
	sem_m = (Semaforo *) mmap(NULL, sizeof(Semaforo), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
	{
		perror("mmap");
		return 30;
	}

    sem_m->S = 1;

	// wait for someone to read it
	//sleep(10);
    printf("Done SettingUp\n");



return 0;
}


/*** end of file ***/