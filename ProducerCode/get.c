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
} Memory;


typedef struct {
    int size;
    int S;
    Memory data[];
} buffer;

typedef struct{
    int msjProducidos;
    double watingTime;
    double waitingSTime;
    double kernelTime;
    pid_t pid;   
} Info;




int main(int argc, char *argv[])
{
    if(argc < 2){
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
    

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

	procInfo.pid = getpid();

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

	while(1){
        int succes = 0;
        if(addr->S == 1){
            addr->S = 0;
            start = clock();
            data.size = addr->size;
	        printf("PID %d: Read from shared memory\n", procInfo.pid);
            printf("BufferSize: %d\n", data.size);
            for(int i = 0; i < addr->size; i++){
                if(addr->data[i].inUse == 0){
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
                    printf("MSG: %s\n", addr->data[i].date);
                }
            }
            if(succes == 0){
                printf("Buffer Lleno\n");
            }
            addr->S = 1;
            end = clock();
            procInfo.kernelTime += ((double) (end - start)) / CLOCKS_PER_SEC;
        }
        printf("Me voy a dormir\n");
        printf("Mensajes Prod: %d, Kernel Time: %f s\n", procInfo.msjProducidos, procInfo.kernelTime);
        sleep(7);
    }
	return 0;
}