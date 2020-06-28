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
#include "../include/printc.h"
#include "../include/struct.h"

#define STORAGE_ID "/SHM_TEST"
#define DATE "%d-%02d-%02d %02d:%02d:%02d"


//Variables globales del consumidor
int enter;
pthread_t thread_id; 
pthread_t thread_id2;
double media;

//Función que obtiene un número random
double U_Random (int random){
    double ran;
    ran =  rand() % 100 ;
    return ran/(100 * random);
}

//Función que obtiene el valor aleatorio de poisson
int possion (int random){
    int lambda = 5, d= 0;
    long double o = 1.0;
    long double w = exp (-lambda);
    while (o >= w){
        double u = U_Random (random);
        o *= u;
        d ++;
    }
    return d-1;
}




//Función que espera a que se precione enter
void *enterfunc(void *vargp){
    while(1){
        enter = getchar();
        return 0;
    }
}


typedef struct{ //Estructura de la información propia del consumidor
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

Pack *global;

//Función que espera a que pase cierto tiempo
void *sleepfunc(void *vargp){
    int p = possion(media);
    clock_t start, end;
    start = clock();
    while(1){
        end = clock();
        int tiempoFinal =((int) (end - start)) / CLOCKS_PER_SEC;
        if(tiempoFinal >= p){
            return 0;
        }else if(global->autodestroy == 1){
            return 0;
        }

    }
}
//Main principal
int main(int argc, char *argv[])
{
    if(argc < 4){
		printf("Missing arguments, please provide buffer name, media time, mode(0 enter mode, 1 poisson mode)\n");
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
    sprintf(g_var, "var_%s", argv[1]); //Nombre del espacio de memoria donde se encontrara las variables globales 

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
    //Set Up Global variables
    fd = shm_open(g_var, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
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
    //Verifica si no hay espacio en el buffer para consumidores
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
    global->consTotal += 1;
	while(1){
        int succes = 0;
        //Verifica que el semáforo esté liberado y que sea el siguiente proceso en lista
        if(sem_m->S == 1 && sem_m->pids[sem_m->index] == consInfo.pid){
            sem_m->S = 0;

            start = clock();
            data.size = addr->size;
            //Verifica que existan mensajes
            printf("índice del mensaje: %d \n",i);
            printf("El mensaje es: %d \n",addr->data[i].processID);
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
                //Verifica si el número mágicod el mensaje es igual al pid módulo 6
                if((addr->data[i].magicNum) == (consInfo.pid % 6)){
                    //Borra el mensaje
                    addr->data[i].inUse = 0;
                    for(int w = i; i<=addr->size;i++){
                        Memory mensaje = addr->data[w];
                        addr->data[w] = addr->data[w+1];
                        addr->data[w+1] = mensaje;
                    }
                    end = clock();
                    consInfo.UserTime +=  ((double) (end - start)) / CLOCKS_PER_SEC;
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
                    //Recoloca los procesos y termina
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
                    //Verifica si es el último proceso en lista y vuelve al primero
                    if(sem_m->index >= sem_m->procCount){//En caso de que al aumentar el indice se salga de la cantidad de procesos en cola
                        printf("Disminuyo\n");
                        sem_m->index = 0;
                    }
                    if(i < addr->size - 1){
                        i += 1;
                    }else{
                        i = 0;
                    }
                }
                
            }else{
                sem_m->index += 1;
                printf("MSG: No hay mensajes disponibles\n");
                //i = 0;
                if(sem_m->index >= sem_m->procCount){//En caso de que al aumentar el indice se salga de la cantidad de procesos en cola
                    printf("Disminuyo\n");
                    sem_m->index = 0;
                }
                if(i < addr->size - 1){
                    i += 1;
                }else{
                    i = 0;
                }
            }

            sem_m->S = 1;
            end = clock();
            consInfo.msjConsumidos += 1;
            consInfo.UserTime +=  ((double) (end - start)) / CLOCKS_PER_SEC;
            consInfo.kernelTime += ((double) (end - start)) / CLOCKS_PER_SEC;
        }
        start = clock(); 
        //Verica si está en modo enter
        if(modo == 0){
            pthread_create(&thread_id, NULL, enterfunc, NULL);
            pthread_join(thread_id, NULL);
        }
        //Verifica si está en modo espera con tiempo igua al número de poisson
        if(modo == 1){   
            pthread_create(&thread_id2, NULL, sleepfunc, NULL);
            pthread_join(thread_id2, NULL);
        }
        //Verifica la bandera autodestroy
        if(global->autodestroy == 1){
            global->numConsAct -= 1;
            end = clock();
            consInfo.watingTime += ((double) (end - start)) / CLOCKS_PER_SEC;;
            global->totKernTime += consInfo.UserTime;
            global->waitingTot += consInfo.watingTime;
            global->totUsrTime += consInfo.UserTime;
            sprintf(msg, "PID: %d Mensajes Leídos: %d, Tiempo de Esperado: %f, Tiempo en kernel: %f\n", consInfo.pid, consInfo.msjConsumidos, consInfo.watingTime, consInfo.UserTime);
            printc(msg, 1);
            return 0;
        }
        end = clock();
        consInfo.watingTime += ((double) (end - start)) / CLOCKS_PER_SEC;;
    }
}