#ifndef _STRUCT_H
#define _STRUCT_H


typedef struct { //Struct de cada segmento del buffer
    int inUse;
    int processID;
    char msg[20];
    char date[50];
    int magicNum;
} Memory;

typedef struct {
    int size;
    Memory data[];
} buffer;

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


#endif