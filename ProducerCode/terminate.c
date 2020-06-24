#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define STORAGE_ID "holis"
//#define STORAGE_SIZE 32
#define DATA "Hello, World! From PID %d"



struct buffer
{
    int size;
    int data[4];
    int S;
};

typedef struct {
    int S;
	int procCount;
	int index;
    int pids[];
} Semaforo;


int main(int argc, char *argv[])
{
	if(argc < 2){
		printf("Missing arguments, please provide buffer name\n");
		return 30;
	}
    int res;
	int fd;
	int len;
	pid_t pid;
	struct buffer *addr;
	struct buffer data;
	Semaforo *sem_m;

	char *sem_msg = malloc(sizeof(char) * (strlen(argv[1]) + 2));

    sprintf(sem_msg, "s_%s", argv[1]);

    size_t size = sizeof(data);

	pid = getpid();

    fd = shm_open(argv[1], O_RDONLY, S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		perror("open");
		return 10;
	}

	// map shared memory to process address space
	addr = (struct buffer *) mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
	{
		perror("mmap");
		return 30;
	}
    // mmap cleanup
	res = munmap(addr, size);
	if (res == -1)
	{
		perror("munmap");
		return 40;
	}

	// shm_open cleanup
	fd = shm_unlink(argv[1]);
	if (fd == -1)
	{
		perror("unlink1");
		return 100;
	}

	fd = shm_open(sem_msg, O_RDONLY, S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		perror("open");
		return 10;
	}

	// map shared memory to process address space
	sem_m = (Semaforo *) mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
	{
		perror("mmap");
		return 30;
	}
    // mmap cleanup
	res = munmap(sem_m, sizeof(Semaforo));
	if (res == -1)
	{
		perror("munmap");
		return 40;
	}

	// shm_open cleanup
	fd = shm_unlink(sem_msg);
	if (fd == -1)
	{
		perror("unlink2");
		return 100;
	}

    printf("Done Cleaning Up\n");

	return 0;

}