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
	//sprintf(data, DATA, pid);

    data.size = 4;

    for(int i = 0; i < 4; i++){
        data.data[i] = i;
    }
    data.S = 1;

	// get shared memory file descriptor (NOT a file)
	fd = shm_open(STORAGE_ID, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		perror("open");
		return 10;
	}

	// extend shared memory object as by default it's initialized with size 0
	res = ftruncate(fd, size);
	if (res == -1)
	{
		perror("ftruncate");
		return 20;
	}

	// map shared memory to process address space
	addr = (struct buffer *) mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
	{
		perror("mmap");
		return 30;
	}

	// place data into memory
	//len = strlen(data) + 1;
	//memcpy(addr, data, len);

    addr->size = data.size;

    addr->S = data.S;

    for(int i = 0; i < 4; i++){
        addr->data[i] = data.data[i];
    }

	// wait for someone to read it
	//sleep(10);
    printf("Done SettingUp\n");
}