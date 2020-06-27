APP = Inicializador


SRC = $(APP).c

CC = gcc
#CFLAGS = -lrt

PROD= ../ProducerCode
CONS= ../consumerCode

LIBS = -lrt



 
APP: 
		$(CC) -o $(APP) $(SRC) $(LIBS)

