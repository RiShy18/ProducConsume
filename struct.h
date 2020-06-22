#ifndef _STRUCT_H
#define _STRUCT_H


typedef struct { //Struct de cada segmento del buffer
    int processID;
    char msg[10];
    char date[50];
} Message;

typedef struct {
    int inUse;
    int processID;
    char msg[10];
    char date[50];
} Buffer;

typedef struct {
    int numSem; //Número de semáforos
    int numCons; //Max de consumidores
    int numProd; //Max de productores

    int msgInBuff; //Mensajes en Buffer
    int totalMsg; //Total de Mensajes
    int deletedCons; //Consumidores borrados

    int autodestroy; //Flag to terminate all
} Pack;    //Variables globales


#endif