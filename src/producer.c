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
#include <math.h>
#include "../include/struct.h"
#include "../include/printc.h"

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

double ran_expo(double lambda){
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) / lambda;
}




int main(int argc, char *argv[])
{
    if(argc < 3){ //Verificar que se hallan introducido los parametros necesarios.
		printf("Missing arguments, please provide buffer name\n");
		return 30;
	}
    char *suc_msg = malloc(sizeof(char) * 500);
    srand((unsigned)time(NULL));
    double sleep_time = ran_expo(atof(argv[2]));
    sprintf(suc_msg, "Tiempo de ejecucion: %f\n", sleep_time);
    printc(suc_msg, 4);

	int fd;
	buffer data;
	buffer *addr;
    Info procInfo;
    ssize_t size = sizeof(data);
    clock_t start, end, slockStart, slockEnd, wTimeStart, wTimeEnd;
    double kernel;
    char date[50];
    char msg[10];

    Semaforo *sem_m;
    Semaforo *sem_glob;

    Pack *globals;

	char *sem_msg = malloc(sizeof(char) * (strlen(argv[1]) + 2));
    char *var_msg = malloc(sizeof(char) * (strlen(argv[1]) + 4));
    char *sg_msg = malloc(sizeof(char) * (strlen(argv[1]) + 3));


    int done_startup = 0;


    sprintf(sem_msg, "s_%s", argv[1]); //Nombre del espacio de memoria donde se encontrara el semaforo del buffer
    sprintf(var_msg, "var_%s", argv[1]);
    sprintf(sg_msg, "sg_%s", argv[1]);


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
	if (sem_m == MAP_FAILED)
	{
		perror("mmap");
		return 30;
	}

    /*------------ Aumenta la cantidad de procesos en la cola del semaforo---------------------*/

    sem_m->procCount +=1;

    /*------------ Inserta el PID en la cola del semaforo---------------------*/

    sem_m->pids[sem_m->procCount - 1] = procInfo.pid;

    /*------------------------ Carga el espacio de las globales en el buffer-----------------------------*/
    fd = shm_open(var_msg, O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		perror("open");
		return 10;
	}

	// map shared memory to process address space
	globals = (Pack *) mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (globals == MAP_FAILED)
	{
		perror("mmap");
		return 30;
	}

    /*------------------------ Carga el espacio del semaforo buffer-----------------------------*/

    fd = shm_open(sg_msg, O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		perror("open");
		return 10;
	}

	// map shared memory to process address space
	sem_glob = (Semaforo *) mmap(NULL, sizeof(Semaforo), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (sem_m == MAP_FAILED)
	{
		perror("mmap");
		return 30;
	}

    /*------------ Aumenta la cantidad de procesos en la cola del semaforo---------------------*/

    sem_glob->procCount +=1;

    /*------------ Inserta el PID en la cola del semaforo---------------------*/

    sem_glob->pids[sem_glob->procCount - 1] = procInfo.pid;

    if(globals->numProd >= globals->numProdAct){
        globals->numProdAct += 1;
        globals->prodTotal +=1;
    }else{
        printf("No mas prod\n");
    }

    int locked = 0;
    int full = 1;



	while(1){
        if(sem_m->S == 1 && sem_m->pids[sem_m->index] == procInfo.pid){
            if(locked){
                slockEnd = clock();
                procInfo.waitingSTime += ((double) (slockEnd - slockStart)) / CLOCKS_PER_SEC;
                locked = 0;
            }
            sem_m->S = 0; //Semaforo del buffer down

            /*------------------------ Instrucciones de la Zona Critica -----------------------------------*/
            start = clock();
            data.size = addr->size;
	        //printf("PID %d: Read from shared memory\n", procInfo.pid);
            //printf("BufferSize: %d\n", data.size);
            full = 1;
            for(int i = 0; i < addr->size; i++){
                if(addr->data[i].inUse == 0){
                    srand(time(0));
                    addr->data[i].magicNum = (rand() % (6 - 0 + 1)) + 0;
                    addr->data[i].processID = (int) procInfo.pid;
                    sprintf(addr->data[i].msg, "PID: %d", procInfo.pid);
                    //memcpy(addr->data[i].msg, msg, strlen(msg) + 1);
                    sprintf(addr->data[i].date, DATE, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                    //memcpy(addr->data[i].date, date, strlen(date) + 1);
                    addr->data[i].inUse = 1;
                    //printf("PID %d: introdujo mensaje en indice: %d\n", procInfo.pid, i);
                    printf("Mensaje: %s \n",addr->data[i].msg);
                    sprintf(suc_msg, "PID %d: introdujo mensaje en indice: %d\n", procInfo.pid, i);
                    printc(suc_msg, 3);
                    procInfo.msjProducidos +=1;
                    full = 0;
                    globals->totalMsg +=1;
                    globals->msgInBuff +=1;
                    break;
                }
            }
            if(full){
                wTimeEnd = clock();
                procInfo.watingTime += ((double) (wTimeEnd - wTimeStart)) / CLOCKS_PER_SEC;
            }
            if(full){
                wTimeStart = clock();
            }

            sem_m->index += 1; //Marca para que el indice de la cola aumente en 1 y el proximo en acceder al semaforo siga el orden
            if(sem_m->index >= sem_m->procCount){//En caso de que al aumentar el indice se salga de la cantidad de procesos en cola
                sem_m->index = 0;
            }

            sem_m->S = 1; //Up del semaforo
            end = clock();
            procInfo.kernelTime += ((double) (end - start)) / CLOCKS_PER_SEC;
        }else{
            slockStart = clock();
            locked = 1;
        }    /*
        for(int i = 0; i < sem_m->procCount; i++){
            printf("PID: %d\n", sem_m->pids[i]);
            printf("siguiente Pid: %d \n",sem_m->pids[sem_m->index]);
        }
        printf("Estado del Sem procCount: %d index: %d\n", sem_m->procCount, sem_m->index);
*/
        if(sem_glob->S == 1){
            sem_glob->S == 0;
            if(globals->autodestroy == 1){
                globals->numProdAct -=1;
                globals->totKernTime += procInfo.kernelTime;
                globals->waitingTot += procInfo.watingTime;
                globals->totUsrTime += procInfo.waitingSTime;
                sprintf(suc_msg, "PID: %d Mensajes producidos: %d, Tiempo de Esperado: %f, Tiempo bloqueado por Semaforos: %f, Tiempo en kernel: %f\n", procInfo.pid, procInfo.msjProducidos, procInfo.watingTime, procInfo.waitingSTime, procInfo.kernelTime);
                printc(suc_msg, 1);
                //printf("PID: %d Mensajes producidos: %d, Tiempo de Esperado: %f, Tiempo bloqueado por Semaforos: %f, Tiempo en kernel: %f\n", procInfo.pid, procInfo.msjProducidos, procInfo.watingTime, procInfo.waitingSTime, procInfo.kernelTime);
                return 0;
            }
            sem_glob->S = 1;
        }
        sleep(sleep_time);
    }
	return 0;
}