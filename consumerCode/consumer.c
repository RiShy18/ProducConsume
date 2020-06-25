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
	buffer data;
	buffer *addr;
    Info consInfo;
    ssize_t size = sizeof(data);
    clock_t start, end;
    double kernel;
    char date[50];
    char msg[500];
    
    Semaforo *sem_m;

    char *sem_msg = malloc(sizeof(char) * (strlen(argv[1]) + 2));

    sprintf(sem_msg, "s_%s", argv[1]); //Nombre del espacio de memoria donde se encontrara el semaforo del buffer
    

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

    sem_m->procCount +=1;

    sem_m->pids[sem_m->procCount - 1] = consInfo.pid;
    int i = 0;
	while(1){
        int succes = 0;
        printf("Consumer pid: %d \n", consInfo.pid);
        printf("Semáforo pid: %d \n", sem_m->pids[sem_m->index]);
        if(sem_m->S == 1 && sem_m->pids[sem_m->index] == consInfo.pid){
            sem_m->S = 0;

            start = clock();
            data.size = addr->size;
	        printf("PID %d: Read from shared memory\n", consInfo.pid);
            printf("BufferSize: %d\n", data.size);
            printf("En uso: %d \n",addr->data[0].inUse);
            if(addr->data[i].inUse != 0){
                printf("Hay mensajes \n");
                printf("Numero mágico: %d \n", addr->data[i].magicNum);
                printf("Modulo 6 de pid: %d \n", (consInfo.pid % 6));

                consInfo.msjConsumidos += 1;
                if((addr->data[i].magicNum) == (consInfo.pid % 6)){
                    printf("Entra al if final");
                    for(int w = i; w < (addr->size - 1); w++){
                        addr->data[w] = addr->data[w+1];
                    }
                    end = clock();
                    consInfo.UserTime +=  ((double) (end - start)) / CLOCKS_PER_SEC;
                    consInfo.msjConsumidos += 1;
                    //disminuir número de consumidores vivos
                    sprintf(msg,"El consumidor número de proceso: %d ha terminado \n", consInfo.pid);
                    printf("\033[1;31m");
                    printf("%s",msg);
                    printf("\033[0m");
                    sprintf(msg,"Suspendido por mensaje con número de llave igual al módulo 6 de su número de proceso. \n");
                    printf("\033[1;31m");
                    printf("%s",msg);
                    printf("\033[0m");
                    sprintf(msg,"Estadísticas básicas: \n");
                    printf("\033[1;32m");
                    printf("%s",msg);
                    printf("\033[0m");
                    sprintf(msg,"Tiempo de espera: %f\n", consInfo.watingTime);
                    printf("\033[1;32m");
                    printf("%s",msg);
                    printf("\033[0m");
                    sprintf(msg,"Tiempo de usuario: %f\n", consInfo.UserTime);
                    printf("\033[1;32m");
                    printf("%s",msg);
                    printf("\033[0m");
                    double tiempoTotal = consInfo.UserTime + consInfo.watingTime;
                    sprintf(msg,"Tiempo total: %f\n",tiempoTotal);
                    printf("\033[1;32m");
                    printf("%s",msg);
                    printf("\033[0m");
                    sprintf(msg,"Mensajes Leídos: %d\n",consInfo.msjConsumidos);
                    printf("\033[1;32m");
                    printf("%s",msg);
                    printf("\033[0m");
                    for(int i = sem_m->index; i < sem_m->procCount; i++){
                        printf("Recolocando procesos \n");
                        sem_m->pids[i] = sem_m->pids[i+1];
                    }
                    sem_m->index = 1;
                    sem_m->procCount -= 1;
                    printf("EL SIGUIENTE PROCESO EN EJECUTARSE ES: %d  Y EL PROCCOUNT ES %d\n",sem_m->pids[sem_m->index], sem_m->procCount);
                    sem_m->S = 1;
                    return 0;
                }else{
                  sem_m->index += 1;
                    sem_m->S = 1;
                    if(sem_m->index >= sem_m->procCount){//En caso de que al aumentar el indice se salga de la cantidad de procesos en cola
                        printf("Disminuyo\n");
                        sem_m->index = 0;
                        if(i < addr->size - 1){
                        printf("Valor de i: %d \n", i);
                            i += 1;
                        }else{
                        printf("Valor de i: %d \n", i);
                            i = 0;
                        }
                    }
                    else if(i < addr->size - 1){
                        printf("Valor de i: %d \n", i);
                        i += 1;
                    }else{
                        printf("Valor de i: %d \n", i);
                        i = 0;
                    }
                }
                
            }else{
                sem_m->index += 1;
                printf("MSG: %s\n", addr->data[0].msg);
                if(sem_m->index >= sem_m->procCount){//En caso de que al aumentar el indice se salga de la cantidad de procesos en cola
                    printf("Disminuyo\n");
                    sem_m->index = 0;
                }
            }

            sem_m->S = 1;
            end = clock();
            consInfo.msjConsumidos += 1;
            consInfo.UserTime +=  ((double) (end - start)) / CLOCKS_PER_SEC;
            consInfo.kernelTime += ((double) (end - start)) / CLOCKS_PER_SEC;
        }
        sleep(7);
        consInfo.watingTime += 7;
    }
}