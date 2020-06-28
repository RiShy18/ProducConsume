finalizador: organize inicializador producer consumer libprintc
		gcc -o excecutables/terminator src/terminate.c lib/libprintc.a -lrt

organize:
		rm -r excecutables; mkdir excecutables

inicializador:
		gcc -o excecutables/inicializador src/Inicializador.c -lrt

libprintc:
		gcc -c src/printc.c
		ar rv lib/libprintc.a printc.o
		ranlib lib/libprintc.a

producer: libprintc
		gcc -o excecutables/producer src/producer.c lib/libprintc.a -lrt -lm

consumer: producer
		gcc -o excecutables/consumer src/consumer.c -lpthread -lrt -lm