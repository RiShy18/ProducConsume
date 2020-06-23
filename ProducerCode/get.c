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
#define DATA "Hello from PID: %d"
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
    Memory data[5];
    int S;
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
	int fd;
	buffer data;
	buffer *addr;
    Info procInfo;
    ssize_t size = sizeof(data);
    clock_t start, end;
    double kernel;

	procInfo.pid = getpid();

	// get shared memory file descriptor (NOT a file)
	fd = shm_open(STORAGE_ID, O_RDWR, S_IRUSR | S_IWUSR);
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
        if(addr->S == 1){
            addr->S = 0;
            start = clock();
            data.size = addr->size;
	        printf("PID %d: Read from shared memory\n", procInfo.pid);
            printf("BufferSize: %d\n", data.size);
            printf("Read MSG: %s\n", addr->data[0].msg);

            sprintf(data.data->msg, DATA, procInfo.pid);

            memcpy(addr->data[0].msg, data.data->msg, strlen(data.data[0].msg) + 1);
            procInfo.msjProducidos +=1;
            addr->S = 1;
            end = clock();
            procInfo.kernelTime += ((double) (end - start)) / CLOCKS_PER_SEC;
            procInfo.msjProducidos += 1;
        }
        printf("Me voy a dormir\n");
        sleep(2);
    }
	return 0;
}