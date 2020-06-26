#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

#define STORAGE_ID "/SHM_TEST"
#define DATE "%d-%02d-%02d %02d:%02d:%02d"



typedef struct { //Struct de cada segmento del buffer
    int inUse;
    int processID;
    char msg[10];
    char date[50];
    int magicNum;
} Memory;

typedef struct {
    int size;
    int S;
    Memory data[];
} buffer;

typedef struct{
    int msjConsumidos;
    double watingTime;
    double UserTime;
    double kernelTime;
    pid_t pid;  
} Info;

typedef struct {
    int S;
    int procCount;
    int index;
    int pids[];
} Semaforo;

typedef struct {
    int numSem; //Número de semáforos
    int numCons; //Max de consumidores
    int numProd; //Max de productores

    int numConsAct; //Número de consumidores actual 

    int msgInBuff; //Mensajes en Buffer
    int totalMsg; //Total de Mensajes
    int deletedCons; //Consumidores borrados

    int autodestroy; //Flag to terminate all
} Pack;    //Variables globales

/*double U_Random ()/* generates a 0 ~ Random number between 1{
double f;
Srand (unsigned) time (NULL ));
F = (float) (rand () % 100 );
/* Printf ("% fn", f );
return f/100;
}


int possion ()/* generates a random number with a Poisson distribution. Lamda is the average number 
{
    int Lambda = 20, k = 0;
    long double p = 1.0;
    long double l = exp (-Lambda);/* it is defined as long double for precision, and exp (-Lambda) is a decimal near 0 
    printf ("%.15Lfn", l );
    while (p> = l){
        double u = U_Random ();
        p * = u;
        k ++;
    }
    return k-1;
}*/

int main(int argc, char *argv[])
{
    if(argc < 2){
		printf("Missing arguments, please provide buffer name\n");
		return 30;
	}
	int fd;
    int res;
	buffer data;
	buffer *addr;
    Info consInfo;
    ssize_t size = sizeof(data);
    clock_t start, end;
    double kernel;
    char date[50];
    char msg[500];
    Pack *global;
    
    Semaforo *sem_m;

    char *sem_msg = malloc(sizeof(char) * (strlen(argv[1]) + 2));
    char *g_var = malloc(sizeof(char) * (strlen(argv[1]) + 4));

    sprintf(sem_msg, "s_%s", argv[1]); //Nombre del espacio de memoria donde se encontrara el semaforo del buffer
    sprintf(g_var, "var_%s", argv[1]); //

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

	consInfo.pid = getpid();
    consInfo.kernelTime = 0;
    consInfo.msjConsumidos = 0;
    //consInfo.waitingSTime = 0;
    consInfo.watingTime = 0;

	// get shared memory file descriptor (NOT a file)
	fd = shm_open(argv[1], O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		perror("open");
		return 10;
	}

	// map shared memory to process address space
	addr = (buffer *) mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
	{
		perror("mmap");
		return 30;
	}


    fd = shm_open(sem_msg, O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		perror("open");
		return 10;
	}

	// map shared memory to process address space
	sem_m = (Semaforo *) mmap(NULL, sizeof(Semaforo), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
	{
		perror("mmap");
		return 30;
	}
        //printf("Nice");
    //Set Up Global variables
    fd = shm_open(g_var, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    printf("Entrando a variables globales");
	if (fd == -1)
	{
		perror("open");
		return 10;
	}
    //printf("Nice");
    // extend shared memory object as by default it's initialized with size 0
	res = ftruncate(fd, size);
	if (res == -1)
	{
		perror("ftruncate");
		return 20;
	}
    // map shared memory to process address space
	global = (Pack *) mmap(NULL, sizeof(Pack), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (global == MAP_FAILED)
	{
		perror("mmap");
		return 30;
	}

    global->autodestroy = 1;
    
    fd = shm_unlink(sem_msg);
	if (fd == -1)
	{
		perror("unlink2");
		return 100;
	}

    printf("Done Cleaning Up\n");

	return 0;
}