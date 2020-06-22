/** @file module.c
 * 
 * @brief Modulo inicializador del productor/consumidor 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) 2018 Barr Group. All rights reserved.
 */
/* ----------------------------------Include Spot------------------------ */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

//#include “module.h”
/*----------------------------------End of Include Spot------------------ */

/*!
 * @brief Inicializador de los productores/consumidores
 *
 * @param[in] bufferSize  Tamaño del Buffer
 * @param[in] bufferName  Nombre del Buffer
 *
 * @return int
 */

    //--------------------------------Variable Spot---------------------------------------
    int numSem; //Número de semáforos
    int numCons; //Max de consumidores
    int numProd; //Max de productores

    int msgInBuff; //Mensajes en Buffer
    int totalMsg; //Total de Mensajes
    int deletedCons; //Consumidores borrados

    int autodestroy; //Flag to terminate all

    //Variables de interacción con el usuario
    char *bufferName;
    int bufferSize;


void *realloc(int *ptr, size_t size);

    char *inputString(FILE* fp, size_t size){
    //The size is extended by the input with the value of the provisional
    char *str;
    int ch;
    size_t len = 0;
    str = realloc(NULL, sizeof(char)*size);//size is start size
    if(!str)return str;
    while(EOF!=(ch=fgetc(fp)) && ch != '\n'){
        str[len++]=ch;
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=16));
            if(!str)return str;
        }
    }
    str[len++]='\0';

    return realloc(str, sizeof(char)*len);
}

    //---------------------------------Function Spot--------------------------------------

struct segment{ //Struct de cada segmento del buffer
    int processID;
    char msg[10];
    char date[50];
};

struct buffer{
    int inUse;
    char Name;
    int size;
    int *body;
};

void createBuf( bufferSize, bufferName){
    printf("Creando un buffer");

}

int main(int argc, char * argv[])
{ 



    //Ingreso el nombre


    printf("Ingrese el nombre del buffer de memoria: ");
    bufferName = inputString(stdin, 10);
    //while((bufferName=getchar()) != '\n' && bufferName != EOF);
    //if(scanf("%c", &bufferName)!=1){
        //printf("Nombre inválido\n");
    //}
    //else{
        printf("El nombre del buffer es %s\n",bufferName);
    //}


    //Ingreso el tamaño
    printf("Ingrese el tamaño del buffer de memoria: ");
    if(scanf("%i", &bufferSize) !=1){
        printf("Número inválido\n");
    }
    else{
        printf("El tamaño del buffer será de %i\n",bufferSize);
    }

    createBuf(bufferSize, &bufferName);
    printf("Buffer creado");





return 0;
}


/*** end of file ***/