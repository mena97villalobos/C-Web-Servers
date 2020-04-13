CURRFILE=$(shell cd ..; pwd)
HEADERS=-I$(CURRFILE)/headers/
CFLAGS=-g -Wall
LIBRARIES=-lm -pthread -lrt

OBJECTS=../commonSrc/argValidator.c ./main.c ../commonSrc/mime.c ../commonSrc/net.c

All :  fullMake clean init serverForks

init:
	mkdir build

fullMake:
	@echo Full make

serverForks: $(OBJECTS)
	@echo Server Forks
	gcc  $(CFLAGS) $(HEADERS) -o build/serverForks $(OBJECTS) $(LIBRARIES)

clean:
	@echo Clean compile
	rm -rf build