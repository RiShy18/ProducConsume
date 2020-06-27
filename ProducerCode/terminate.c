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

typedef struct {
    int numSem; //Número de semáforos
    int numCons; //Max de consumidores
    int numProd; //Max de productores

    int numConsAct; //Número de consumidores actual
    int numProdAct;

	int consTotal;
    int prodTotal;

    int msgInBuff; //Mensajes en Buffer
    int totalMsg; //Total de Mensajes
    int deletedCons; //Consumidores borrados

    int autodestroy; //Flag to terminate all
} Pack;    //Variables globales


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
	Semaforo *sem_glob;

	Pack *global_vars;

	char *sem_msg = malloc(sizeof(char) * (strlen(argv[1]) + 2));
	char *var_msg = malloc(sizeof(char) * (strlen(argv[1]) + 4));
	char *sg_msg = malloc(sizeof(char) * (strlen(argv[1]) + 3));

    sprintf(sem_msg, "s_%s", argv[1]);
	sprintf(var_msg, "var_%s", argv[1]);
	sprintf(sg_msg, "sg_%s", argv[1]);


	printf("%s\n",var_msg);

    size_t size = sizeof(data);

	pid = getpid();

	fd = shm_open(var_msg, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

	if (fd == -1)
	{
		perror("open");
		return 10;
	}
    //printf("Nice");
    // extend shared memory object as by default it's initialized with size 0
	res = ftruncate(fd, sizeof(Pack));
	if (res == -1)
	{
		perror("ftruncate");
		return 20;
	}
    // map shared memory to process address space
	global_vars = (Pack *) mmap(NULL, sizeof(Pack), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (global_vars == MAP_FAILED)
	{
		perror("mmap");
		return 30;
	}

	global_vars->autodestroy = 1;

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

	fd = shm_open(var_msg, O_RDONLY, S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		perror("open3");
		return 10;
	}

	// map shared memory to process address space
	global_vars = (Pack *) mmap(NULL, sizeof(Pack), PROT_READ, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
	{
		perror("mmap");
		return 30;
	}
    // mmap cleanup
	res = munmap(global_vars, sizeof(Pack));
	if (res == -1)
	{
		perror("munmap");
		return 40;
	}

	// shm_open cleanup
	fd = shm_unlink(var_msg);
	if (fd == -1)
	{
		perror("unlink3");
		return 100;
	}

	fd = shm_open(sg_msg, O_RDONLY, S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		perror("open");
		return 10;
	}

	// map shared memory to process address space
	sem_glob = (Semaforo *) mmap(NULL, sizeof(Semaforo), PROT_READ, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
	{
		perror("mmap");
		return 30;
	}
    // mmap cleanup
	res = munmap(sem_glob, sizeof(Semaforo));
	if (res == -1)
	{
		perror("munmap");
		return 40;
	}

	// shm_open cleanup
	fd = shm_unlink(sg_msg);
	if (fd == -1)
	{
		perror("unlink2");
		return 100;
	}

    printf("Done Cleaning Up\n");

	printc("<I> Activity Summary: \n\n",4);

    printc("\t- Mensajes Totales: ",4);
    printf("%d\n", global_vars->totalMsg);

    printc("\t- Mensajes en el buffer: ",6);
    printf("%d\n", global_vars->msgInBuff);

    printc("\t- Total de Productores: ",6); //Si tienen otra variable que los cuente lo cambian v:
    printf("%d\n", global_vars->prodTotal);

    printc("\t- Total de Consumidores: ",6); //Si tienen otra variable que los cuente lo cambian v:
    printf("%d\n", global_vars->consTotal);

    printc("\t- Consumidores eliminados por llave: ",6);
    printf("%d\n", global_vars->deletedCons);
/*
    printc("\t- Tiempo esperado total: ",6);        //Mae chizco muerdase con los tiempos v:
    printf("%.2f minutos\n", consInfo.watingTime);

    //printc("\t- Tiempo bloqueado total: ",6);
    //printf("%.2f minutos\n", Variables[0].totalBloq/60);

    printc("\t- Tiempo usuario total: ",6);
    printf("%.2f minutos\n", consInfo.UserTime);

    printc("\t- Tiempo kernel total: ",6);
    printf("%.2f minutos\n", consInfo.kernelTime);
*/
	return 0;

}
/*
;
; 		_
;	.__(.)<  (CUACK)
;	\____)
; ____________________________
;
*/