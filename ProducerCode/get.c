#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include   <sys/types.h>
#include   <sys/ipc.h>
#include   <sys/sem.h>
#include   <errno.h>

#define STORAGE_ID "/SHM_TEST"
//#define STORAGE_SIZE 32

struct buffer
{
    int size;
    int data[4];
    int S;
};
/*
* Provee un interface mas sencillo de entender que las llamadas a sistema
* de semaforos de System V. Hay 7 rutinas disponibles:
*
*   id = semCreate(key, initval);   # Crear con un valor inicial o abrir.
*   id = semOpen(key);               # Abrir (debe existir ya)
*        semWait(id);                 # espera = P = down en 1
*        semSignal(id);               # senal  = V = up   en 1
*        semOp(id, cantidad);        # espera   si (cantidad < 0)
*                                            # senal    si (cantidad > 0)
*        semClose(id);                  # cierra
*        semRm(id);                     # destruye (borra)
*
* Se disegna un semaforo soportado por un conjunto de tres, dos de ellos
* auxiliares. (Los semaforos se crean por arrays)
*   - El primer miembro, [0], es el valor real del semaforo.
*   - El segundo miembro, [1], es un contador utilizado para conocer
*     si todos los procesos han acabado con el semaforo. El contador
*     se inicializa con un numero grande (BIGCOUNT) y se decrementa cada
*     vez que se crea o abre, y se incrementa en cada cierre.
*
*     De esta forma se puede "ajustar" la caracteristica de System V
*     de forma que se tenga en cuenta cualquier proceso que salga
*     sin llamar a semClose(). A pesar de ello, no ahuda mucho si el
*     ultimo proceso sale sin cerrar el semaforo, ya que no hay forma
*     de destruir el semaforo, pero puede ayudar si acaba (intencional
*     o no intencionalmente) cualquier otro proceso diferente del ultimo.
*   - El tercer miembro, [2], del conjunto de semaforos se utiliza para
*     bloquear las secciones criticas en semCreate() y semClose().
*/

void   semOp(int, int);
int    semCreate(key_t, int);
int    semOpen(key_t);
void   semRm(int);
void   semClose(int);
void   semWait(int);
void   semSignal(int);

#define   BIGCOUNT   10000 /* Valor inicial para el contador de procesos */

/* Define los arrays de operaciones del semaforo para llamadas a
* semop().
*/
static struct sembuf   op_lock[2] = {
   2, 0, 0,         /* espera para [2] (bloqueo) sea igual 0
                     * despues incrementa [2] en 1 - esto lo bloquea */
   2, 1, SEM_UNDO   /* UNDO para liberar el bloqueo si el proceso sale
                     * antes de desbloquear explicitamente */
};

static struct sembuf   op_endcreate[2] = {
   1, -1, SEM_UNDO, /* decrementa [1] (contador de procesos) con undo en
                     * caso de finalizar */
                    /* UNDO para ajustar el contador de procesos en caso de
                     * acabar antes de llamar explicitamente a semClose() */
   2, -1, SEM_UNDO  /* despues decrementa [2] (bloqueo) de vuelta a 0 */
};

static struct sembuf   op_open[1] = {
   1, -1, SEM_UNDO   /* decrementa [1] (contador de proceso) con undo en
                      * caso de finalizar */
};

static struct sembuf   op_close[3] = {
   2, 0, 0,          /* espera hasta que [2] (bloqueo) sea igual a 0 */
   2, 1, SEM_UNDO,   /* despues incrementa [2] en 1 - esto lo bloquea */
   1, 1, SEM_UNDO    /* despues incrementa [1] (contador de procesos) */
};

static struct sembuf   op_unlock[1] = {
   2, -1, SEM_UNDO   /* decrementa [2] (bloqueo) de vuelta a 0 */
};

static struct sembuf   op_op[1] = {
   0, 99, SEM_UNDO   /* decrementa o incrementa [0] con undo en caso de
                      * finalizar */
                     /* El 99 se substituye con la cantidad real que hay
                      * que substraer (positiva o negativa) */
};

/****************************************************************************
* Crea un semaforo con un valor inicial especificado.
* Si el semaforo existe, no se inicializa (por su puesto).
* Se devuelve la identidad del semaforo si todo va bien, si no -1.
*/
int semCreate(key_t key, int initval) {
   register int      id, semval;
   union semun {
      int               val;
      struct semid_ds   *buf;
      ushort            *array;
   } semctl_arg;

   if (key == IPC_PRIVATE)
      return(-1);   /* no utilizable para semaforos privados */

   else if (key == (key_t) -1)
      return(-1);   /* probablemente una llamada erronea anterior a ftok() */

deNuevo:
   if ( (id = semget(key, 3, 0666 | IPC_CREAT)) < 0)
      return(-1);   /* problemas de permisos o tablas llenas */

   /* Cuando se crea el semaforo, sabemos que el valor de todos los
   * miembros es 0.
   *
   * Bloquear el semaforo esperando a que [2] sea 0, e incrementarlo.
   *
   * Hay una condicion de carrera: Cabe la posibilidad de que entre el
   * semget() de arriba y el semop() de abajo, otro proceso pueda llamar
   * a semClose() que puede borrar el semaforo si el ultimo lo esta
   * usando.
   *
   * Ademas, se maneja la condicion de error sobre el identificador.
   * Si esto ocurre, se vuelve atras y se intenta crear de nuevo. 
   */

   if (semop(id, &op_lock[0], 2) < 0) {
      if (errno == EINVAL)
         goto deNuevo;  
      fprintf(stderr, "semCreate: no puedo bloquear\n");
   }

   /* Obtener el valor del contador de procesos. Si es igual a 0,
   * entonces ninguno ha inicializado el semaforo aun.
   */
   if ( (semval = semctl(id, 1, GETVAL, 0)) < 0)
      fprintf(stderr, "semCreate: no puedo realizar GETVAL\n");

   if (semval == 0) {
      /* Podriamos inicializar mediante SETALL, pero podria borrar el
      * ajuste del valor que se realizo cuando se bloqueo el semaforo antes.
      * En su lugar, se hacen dos llamadas al sistema para inicializar
      * [0] y [1].
      */

      semctl_arg.val = initval;
      if (semctl(id, 0, SETVAL, semctl_arg) < 0)
         fprintf(stderr, "semCreate: puedo SETVAL[0]\n");

      semctl_arg.val = BIGCOUNT;
      if (semctl(id, 1, SETVAL, semctl_arg) < 0)
         fprintf(stderr, "semCreate: puedo SETVAL[1]\n");
   }

   /* Decrementar el contador de procesos y desbloquear.
   */

   if (semop(id, &op_endcreate[0], 2) < 0)
      fprintf(stderr, "semCreate: no puedo acabar semCreate()\n");

   return(id);
}

/****************************************************************************
* Abre un semaforo que debe existir ya.
* Esta funcion deberia de usarse, en vez de semCreate(), si en la llamada
* se sabe que el semaforo deberia ya existir. Por ejemplo un cliente
* de un par cliente-servidor podria utilizarla, si es responsabilidad del
* servidor crear el semaforo.
* Se vuelve la identidad del semaforo si va bien, si no -1.
*/
int semOpen(key_t key) {
   register int   id;

   if (key == IPC_PRIVATE)
      return(-1);   /* no utilizable para semaforos privados */

   else if (key == (key_t) -1)
      return(-1);   /* probablemente una llamada erronea anterior a ftok() */

   if ( (id = semget(key, 3, 0)) < 0)
      return(-1);   /* no existe o las tablas estan llenas */

   /* Decrementa el contador de procesos. No necesitamos un bloqueo
   *  para hacer esto.
   */
   if (semop(id, &op_open[0], 1) < 0)
      fprintf(stderr, "semOpen: no puedo abrir\n");

   return(id);
}

/****************************************************************************
* Borrar un semaforo.
* Se supone que esta llamada se realiza desde un servidor en operaciones como
* apagarServidor ... No importa si los otros procesos estan usandolo o no.
* El resto de los procesos deberian emplear semClose().
*/
void semRm(int id)
{
   if (semctl(id, 0, IPC_RMID, 0) < 0)
      fprintf(stderr, "semRm: no puedo borrar semaforo (IPC_RMID)\n");
}

/****************************************************************************
* Cerrar el semaforo.
* Funcion por proceso que decrementa el numero de procesos activos en el
* semaforo. Se emplea al salir. Si el proceso es el ultimo destruye el
* semaforo.
*/

void semClose(int id) {
   register int   semval;

   /* En primer lugar bloquear el recurso semaforo e incrementar el contador
   * de procesos [1].
   */
   if (semop(id, &op_close[0], 3) < 0)
      fprintf(stderr, "semClose: no puedo bloquer en semClosep\n");

   /* Comprobar si el valor leido es la ultima referencia al semaforo.
    */
   if ( (semval = semctl(id, 1, GETVAL, 0)) < 0)
      fprintf(stderr, "semClose: no puedo realizar GETVAL\n");

   if (semval > BIGCOUNT)
      fprintf(stderr, "< BIGCOUNT>>\n");
   else if (semval == BIGCOUNT)
      semRm(id);
   else if (semop(id, &op_unlock[0], 1) < 0)
         fprintf(stderr, "semClose: no puedo desbloquear\n"); /* desbloqueo */
}

/****************************************************************************
* Espera hasta que el valor del semaforo sea mayor que 0, entonces
* decrementa en 1 y vuelve. Operador wait, DOWN (Tanenbaum) o P (Dijkstra).
*/
void semWait(int id) {
   semOp(id, -1);
}

/****************************************************************************
* Incrementar el semaforo en 1. Operador segnal, UP (Tanenbaum) o
* V (Dijkstra).
*/
void semSignal(int id) {
   semOp(id, 1);
}

/****************************************************************************
* Operacion generica de semaforo:
* incrementar o decrementar cierta cantidad positiva o negativa, distinta
* de cero.
*/
void semOp(int id, int value) {
   if ( (op_op[0].sem_op = value) == 0)
      (void) fprintf(stderr, "semOp: 'valor' no puede ser 0\n");

   if (semop(id, &op_op[0], 1) < 0)
      (void) fprintf(stderr, "semOp: error\n");
}

int main(int argc, char *argv[])
{
	int res;
	int fd;
	struct buffer data;
	pid_t pid;
	struct buffer *addr;
    ssize_t size = sizeof(data);

	pid = getpid();

	// get shared memory file descriptor (NOT a file)
	fd = shm_open(STORAGE_ID, O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		perror("open");
		return 10;
	}

	// map shared memory to process address space
	addr = (struct buffer *) mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
	{
		perror("mmap");
		return 30;
	}

	while(1){
        // place data into memory
	    //memcpy(data, addr, STORAGE_SIZE);

        if(addr->S == 1){
            addr->S = 0;
            data.size = addr->size;
            for(int i = 0; i < 4; i++){
                data.data[i] = addr->data[i];
            }
	        printf("PID %d: Read from shared memory\n", pid);
            printf("BufferSize: %d\n", data.size);

            for(int i = 0; i < 4; i++){
                printf("%d\n", addr->data[i]);
            }
            sleep(5);
            addr->S = 1;
        }
        printf("Me voy a dormir\n");
        sleep(2);
    }
	return 0;
}