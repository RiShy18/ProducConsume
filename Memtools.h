#ifndef _MEMTOOLS_H
#define _MEMTOOLS_H

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int isEmpty(Buffer *Buffer, int size);
int isFull(Buffer *Buffer, int size);
int isFree(Buffer *Buffer, int size);
int freeAll(int shmid);
int getSpot(Buffer *Buffer, int processID);
int newMem(int *shmid, key_t key, int size, Buffer **Memory);
int newPack(int *shmid, Pack **Mem);


#endif