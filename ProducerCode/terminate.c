#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define STORAGE_ID "/SHM_TEST"
//#define STORAGE_SIZE 32
#define DATA "Hello, World! From PID %d"



struct buffer
{
    int size;
    int data[4];
    int S;
};



int main(int argc, char *argv[])
{
    int res;
	int fd;
	int len;
	pid_t pid;
	struct buffer *addr;
	struct buffer data;

    size_t size = sizeof(data);

	pid = getpid();

    fd = shm_open(STORAGE_ID, O_RDONLY, S_IRUSR | S_IWUSR);
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
	fd = shm_unlink(STORAGE_ID);
	if (fd == -1)
	{
		perror("unlink");
		return 100;
	}

    printf("Done Cleaning Up\n");

	return 0;

}