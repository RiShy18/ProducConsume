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
#include <pthread.h> 
#include <math.h>

#define STORAGE_ID "/SHM_TEST"
#define DATE "%d-%02d-%02d %02d:%02d:%02d"



int enter;
pthread_t thread_id; 
pthread_t thread_id2;
double media;

double U_Random (int random){
    double f;
    f =  rand() % 100 ;
    return f/(100 * random);
}

int possion (int random)
{
    int Lambda = 5, k = 0;
    long double p = 1.0;
    long double l = exp (-Lambda);
    while (p >= l){
        double u = U_Random (random);
        p *= u;
        k ++;
    }
    return k-1;
}

typedef struct { //Struct de cada segmento del buffer
    int inUse;
    int processID;
    char msg[10];
    char date[50];
    int magicNum;
} Memory;

void *enterfunc(void *vargp){
    while(1){
        enter = getchar();
        return 0;
    }
}


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
    int numProdAct;

    int msgInBuff; //Mensajes en Buffer
    int totalMsg; //Total de Mensajes
    int deletedCons; //Consumidores borrados
    int prodTotal;
    int consTotal;
    double waitingTot;
    double bloquedTot;
    double totUsrTime;
    double totKernTime;

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
Pack *global;

void *sleepfunc(void *vargp){
    int p = possion(media);
    clock_t start, end;
    start = clock();
    while(1){
        end = clock();
        int tiempoFinal =((int) (end - start)) / CLOCKS_PER_SEC;
        if(tiempoFinal >= p){
            printf("El tiempo final es %d  \n", tiempoFinal);
            return 0;
        }else if(enter == 10){
            printf("Se pulsó enter \n");
            enter = 0;
            return 0;
        }else if(global->autodestroy == 1){
            return 0;
        }

    }
}

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
    media = atof(argv[2]);
    int modo = atof(argv[3]); 
    
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
    
	global = (Pack *) mmap(NULL, sizeof(Pack), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (global == MAP_FAILED)
	{
		perror("mmap");
		return 30;
	}

    sem_m->procCount +=1;

    sem_m->pids[sem_m->procCount - 1] = consInfo.pid;
    int i = 0;
    printf("Número de consumidores actuales es: %d \n", global->numConsAct);
    if(global->numCons == 0){
        printf("Número máximo de consumidores alcanzado");
        while(1){
            pthread_create(&thread_id, NULL, enterfunc, NULL);
            pthread_create(&thread_id2, NULL, sleepfunc, NULL);
            //pthread_join(thread_id, NULL);
            pthread_join(thread_id2, NULL);
            if(global->numCons > 0){
                printf("Se ha liberado un espacio");
                break;
            }
        }
    }
    global->numCons -= 1;
    global->numConsAct += 1;
    printf("Numero de consumidores actuales es: %d \n", global->numConsAct);
	while(1){
        int succes = 0;
        printf("El número de consumidores globales es: %d \n", global->numCons);
        if(sem_m->S == 1 && sem_m->pids[sem_m->index] == consInfo.pid){
            sem_m->S = 0;

            start = clock();
            data.size = addr->size;
	        printf("PID %d: Read from shared memory\n", consInfo.pid);
            printf("BufferSize: %d\n", data.size);
            printf("En uso: %d \n",addr->data[0].inUse);
            if(addr->data[i].inUse != 0){
                sprintf(msg,"El indice del mensaje actual es: %d \n", i);
                printf("\033[1;32m");
                printf("%s",msg);
                printf("\033[0m");
                sprintf(msg,"El número de consumidores actuales es: %d \n",global->numConsAct);
                printf("\033[1;32m");
                printf("%s",msg);
                printf("\033[0m");
                sprintf(msg,"El número de productores actuales es: %d \n", global->numProdAct);
                printf("\033[1;32m");
                printf("%s",msg);
                printf("\033[0m");                
                sprintf(msg,"El mensaje leído dice: %s \n", addr->data[i].msg);
                printf("\033[1;32m");
                printf("%s",msg);
                printf("\033[0m");
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
                    global->numCons += 1;
                    global->numConsAct -= 1;
                    sem_m->index = 1;
                    sem_m->procCount -= 1;
                    sem_m->S = 1;
                    return 0;
                }else{
                  sem_m->index += 1;
                    sem_m->S = 1;
                    if(sem_m->index >= sem_m->procCount){//En caso de que al aumentar el indice se salga de la cantidad de procesos en cola
                        printf("Disminuyo\n");
                        sem_m->index = 0;
                        if(i < addr->size - 1){
                            i += 1;
                        }else{
                            i = 0;
                        }
                    }
                    else if(i < addr->size - 1){
                        i += 1;
                    }else{
                        i = 0;
                    }
                }
                
            }else{
                sem_m->index += 1;
                printf("MSG: No hay mensajes disponibles\n");
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
        start = clock(); 
        if(modo == 0){
            pthread_create(&thread_id, NULL, enterfunc, NULL);
            pthread_join(thread_id, NULL);
        }
        if(modo == 1){   
            pthread_create(&thread_id2, NULL, sleepfunc, NULL);
            pthread_join(thread_id2, NULL);
        }
        //pthread_join(thread_id, NULL);
        if(global->autodestroy == 1){
            //global->numCons = 0;
            global->numConsAct -= 1;
            return 0;
        }
        end = clock();
        consInfo.watingTime += ((double) (end - start)) / CLOCKS_PER_SEC;;
    }
}