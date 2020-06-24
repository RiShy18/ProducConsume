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
#include <math.h>

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
    

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

	consInfo.pid = getpid();

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
    
    int i = 0;
	while(1){
        int succes = 0;
        if(addr->S == 1){
            addr->S = 0;
            start = clock();
            data.size = addr->size;
	        printf("PID %d: Read from shared memory\n", consInfo.pid);
            printf("BufferSize: %d\n", data.size);
            printf("En uso: %d \n",addr->data[0].inUse);
            printf("valor de i %d", i);
            if(addr->data[i].inUse != 0){
                printf("Hay mensajes \n");
                printf("Numero mágico: %d \n", addr->data[0].magicNum);
                printf("Modulo 6 de pid: %d \n", (consInfo.pid % 6));
                consInfo.msjConsumidos += 1;
                if((addr->data[0].magicNum) == (consInfo.pid % 6)){
                    for(int w = 0; w < (addr->size - 1); w++){
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
                    printf("Suspendido por mensaje con número de llave igual al módulo 6 de su número de proceso. \n");
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
                    return 0;
                }else{
                    printf("Entra acá \n");
                    i++;
                    printf("valor de i = %d \n",i);
                    if(i == addr->size + 1){
                        i = 0;
                    }
                }
                
            }else{
                printf("MSG: %s\n", addr->data[0].date);
            }
            addr->S = 1;
            end = clock();
            consInfo.msjConsumidos += 1;
            consInfo.UserTime +=  ((double) (end - start)) / CLOCKS_PER_SEC;
            consInfo.kernelTime += ((double) (end - start)) / CLOCKS_PER_SEC;
        }
        sleep(7);
        consInfo.watingTime += 7;
    }
}