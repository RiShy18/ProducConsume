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
//#define DATA "PID: %d"
#define DATE "%d-%02d-%02d %02d:%02d:%02d"
//#define STORAGE_SIZE 32
/*
struct buffer
{
    int size;
    int data[4];
    int S;
};*/

typedef struct { //Struct de cada segmento del buffer
    int inUse;
    int processID;
    char msg[10];
    char date[50];
    int magicNum;
} Memory;


typedef struct {
    int size;
    Memory data[];
} buffer;

typedef struct{
    int msjProducidos;
    double watingTime;
    double waitingSTime;
    double kernelTime;
    pid_t pid;   
} Info;

typedef struct {
    int S;
    int procCount;
    int index;
    int pids[];
} Semaforo;




int main(int argc, char *argv[])
{
    if(argc < 2){ //Verificar que se hallan introducido los parametros necesarios.
		printf("Missing arguments, please provide buffer name\n");
		return 30;
	}
	int fd;
	buffer data;
	buffer *addr;
    Info procInfo;
    ssize_t size = sizeof(data);
    clock_t start, end;
    double kernel;
    char date[50];
    char msg[10];

    Semaforo *sem_m;

	char *sem_msg = malloc(sizeof(char) * (strlen(argv[1]) + 2));

    sprintf(sem_msg, "s_%s", argv[1]); //Nombre del espacio de memoria donde se encontrara el semaforo del buffer
    

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

	procInfo.pid = getpid();
    procInfo.kernelTime = 0;
    procInfo.msjProducidos = 0;
    procInfo.waitingSTime = 0;
    procInfo.watingTime = 0;

    /*------------------------ Carga el espacio del buffer-----------------------------*/

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

    /*------------------------ Carga el espacio del semaforo buffer-----------------------------*/

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

    /*------------ Aumenta la cantidad de procesos en la cola del semaforo---------------------*/

    sem_m->procCount +=1;

    /*------------ Inserta el PID en la cola del semaforo---------------------*/

    sem_m->pids[sem_m->procCount - 1] = procInfo.pid;

	while(1){
        int succes = 0;
        if(sem_m->S == 1 && sem_m->pids[sem_m->index] == procInfo.pid){
            sem_m->S = 0; //Semaforo del buffer down

            /*------------------------ Instrucciones de la Zona Critica -----------------------------------*/
            start = clock();
            data.size = addr->size;
	        //printf("PID %d: Read from shared memory\n", procInfo.pid);
            //printf("BufferSize: %d\n", data.size);
            for(int i = 0; i < addr->size; i++){
                if(addr->data[i].inUse == 0){
                    addr->data[i].magicNum = procInfo.pid % 6;
                    addr->data[i].processID = (int) procInfo.pid;
                    sprintf(msg, "PID: %d", procInfo.pid);
                    memcpy(addr->data[i].msg, msg, strlen(msg) + 1);
                    sprintf(date, DATE, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                    memcpy(addr->data[i].date, date, strlen(date) + 1);
                    addr->data[i].inUse = 1;
                    printf("PID %d: introdujo mensaje en indice: %d\n", procInfo.pid, i);
                    procInfo.msjProducidos +=1;
                    succes = 1;
                    break;
                }else{
                    //printf("MSG: %s\n", addr->data[i].date);
                }
            }
            if(succes == 0){
                printf("Buffer Lleno\n");
            }

            sem_m->index += 1; //Marca para que el indice de la cola aumente en 1 y el proximo en acceder al semaforo siga el orden
            printf("Aumento\n");
            if(sem_m->index >= sem_m->procCount){//En caso de que al aumentar el indice se salga de la cantidad de procesos en cola
                printf("Disminuyo\n");
                sem_m->index = 0;
            }

            sem_m->S = 1; //Up del semaforo
            end = clock();
            procInfo.kernelTime += ((double) (end - start)) / CLOCKS_PER_SEC;
        }
        for(int i = 0; i < sem_m->procCount; i++){
            printf("PID: %d\n", sem_m->pids[i]);
        }
        printf("Estado del Sem procCount: %d index: %d\n", sem_m->procCount, sem_m->index);

        sleep(7);
    }
	return 0;
}