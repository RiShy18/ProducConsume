/** @file Inicializador.c
 * 
 * @brief Modulo inicializador del productor/consumidor 
 *
 * @par       
 * COPYRIGHT NOTICE: (R) 2020 Barr Group. GNU licensed.
 */
/* ----------------------------------Include Spot------------------------ */

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "Memtools.h"
#include "struct.h"

/* Inclusion of system and local header files goes here */



//---------------------------------Function Spot--------------------------------------

#define ARRAY_SIZE   4000
#define MALLOC_SIZE 10000
#define SHM_MODE    (SHM_R | SHM_W)

#define TRUE 1
#define FALSE 0

/* #define and enum statements go here */


//---------------------------------Function Spot--------------------------------------

/**
 * @brief Verifica si la memoria esta vacia
 * @return Retorna 0 no esta vacia
 * @return Retorna 1 vacia
*/
int isEmpty(Buffer *Mem, int size){
  for (int i = 0; i < size; i++){
    if(Mem[i].inUse){
      return 0;
    }
  }
  return 1;
}
/**
 * @brief Verifica si la memoria esta llena
 * @return Retorna 0 si no esta llena
 * @return Retorna 1 si esta llena
*/
int isFull(Buffer *Mem, int size){
  for (int i = 0; i < size; i++){
    if(!(Mem[i].inUse)){
      return 0;
    }
  }
  return 1;
}
/**
 * @brief Verifica si la memoria esta llena
 * @return Retorna 0 no hay nadie libre
 * @return Retorna el índice del espacio libre
*/
int isFree(Buffer *Mem, int size){
    for(int i=0; i < size; i++){
        if(!(Mem[i].inUse)){
            return i;
        }
    }
    return 0;
}

/**
 * @brief Libera el buffer
 * @return 0 si no se pudo
 * @return 1 si se logró
*/
int freeAll(int shmid){
    if (shmctl(shmid, IPC_RMID, 0) < 0){
        return 0;
    }else{
        return 1;
    }
}

/**
 * @brief Obtiene un Buffer con un ID
 * @return Devuelve 1 si carga el buffer
 * @return Devuelve 0 si hubo un error
*/
int getSpot(Buffer **Mem, int processID){
    if((*Mem=( Buffer *)shmat(processID, (char *)0, 0)) == (void *) -1){
        printf("Error al obtener el buffer\n");
        return 0;
    }
    return 1;

}

/**
 * @brief Crea el buffer en memoria compartida
 * @return Devuelve 1 si crea el buffer
 * @return Devuelve 0 si hubo un error
*/
int newMem(int *shmid, key_t key, int size, Buffer **Mem){
    if ((*shmid=shmget(key, sizeof(Buffer)*size, 0777 | IPC_CREAT))<0) {
    printf("Error al obtener del buffer\n");
    return 0;
   } 
   if ((*Mem=( Buffer *)shmat(*shmid, (char *)0, 0)) == (void *) -1) {
    printf("Error en la dirección del buffer\n");
    return 0;
   }
   return 1;
}
/**
 * @brief Crea el el pack de variables compartidas
 * @return Devuelve 1 si crea el pack
 * @return Devuelve 0 si hubo un error
*/
int newPack(int *shmid, Pack **Mem){
  int shmid;
  key_t key = ftok ("/bin/ls", 33);
	if (key == -1){
		printf("No se consigue la clave para las variables globales\n");
		return 0;
	}
  if ((shmid=shmget(key, sizeof(Pack), 0777 | IPC_CREAT))<0) {
    printf("Error al obtener del buffer\n");
    return 0;
   } 
   if ((*Mem=( Pack *)shmat(shmid, (char *)0, 0)) == (void *) -1) {
    printf("Error en la direccion del buffer\n");
    return 0;
   }
   return 1;
}
