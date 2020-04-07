CURRFILE=$(shell cd ..; pwd)
HEADERS=-I$(CURRFILE)/headers/
CFLAGS=-g -Wall
LIBRARIES=-lm -pthread -lrt

OBJECTS=../commonSrc/argValidator.c ./client.c ../commonSrc/mime.c ../commonSrc/net.c

All :  fullMake clean init client

init:
	mkdir build

fullMake:
	@echo Full make

client: $(OBJECTS)
	@echo Client
	gcc  $(CFLAGS) $(HEADERS) -o build/client $(OBJECTS) $(LIBRARIES)

clean:
	@echo Clean compile
	rm -rf build