CURRFILE=$(shell cd ..; pwd)
HEADERS=-I$(CURRFILE)/headers/
CFLAGS=-g -Wall
LIBRARIES=-lm -pthread -lrt

OBJECTS=../commonSrc/argValidator.c ./cliente.c ../commonSrc/mime.c ../commonSrc/net.c

All :  fullMake clean init cliente

init:
	mkdir build

fullMake:
	@echo Full make

cliente: $(OBJECTS)
	@echo Client
	gcc  $(CFLAGS) $(HEADERS) -o build/cliente $(OBJECTS) $(LIBRARIES)

clean:
	@echo Clean compile
	rm -rf build