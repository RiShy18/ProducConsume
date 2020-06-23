#ifndef _MEMTOOLS_H
#define _MEMTOOLS_H

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int isEmpty(buffer *Buffer, int size);
int isFull(buffer *Buffer, int size);
int isFree(buffer *Buffer, int size);
int freeAll(int shmid);
int getSpot(buffer *Buffer, int processID);
int newMem(int *shmid, key_t key, int size, buffer **Memory);
int newPack(int *shmid, Pack **Mem);


#endif