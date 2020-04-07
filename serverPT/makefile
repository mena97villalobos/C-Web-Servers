CURRFILE=$(shell pwd)
HEADERS=-I$(CURRFILE)/headers/
CFLAGS=-g -Wall
LIBRARIES=-lm -pthread -lrt

OBJECTS=./src/argValidator.c ./src/main.c ./src/mime.c ./src/net.c

All :  fullMake clean init starter

init:
	mkdir build

fullMake:
	@echo Full make

starter: $(OBJECTS)
	@echo Server
	gcc  $(CFLAGS) $(HEADERS) -o build/serverPT.o $(OBJECTS) $(LIBRARIES)

clean:
	@echo Clean compile
	rm -rf build
