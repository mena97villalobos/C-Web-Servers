CURRFILE=$(shell cd ..; pwd)
HEADERS=-I$(CURRFILE)/headers/
CFLAGS=-g -Wall
LIBRARIES=-lm -pthread -lrt

OBJECTS=../commonSrc/argValidator.c ./main.c ../commonSrc/mime.c ../commonSrc/net.c

All :  fullMake clean init server-secuencial

init:
	mkdir build

fullMake:
	@echo Full make

server-secuencial: $(OBJECTS)
	@echo Server Secuencial
	gcc  $(CFLAGS) $(HEADERS) -o build/server-secuencial $(OBJECTS) $(LIBRARIES)

clean:
	@echo Clean compile
	rm -rf build
