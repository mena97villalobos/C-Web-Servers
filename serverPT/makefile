CURRFILE=$(shell cd ..; pwd)
HEADERS=-I$(CURRFILE)/headers/
CFLAGS=-g -Wall
LIBRARIES=-lm -pthread -lrt

OBJECTS=../commonSrc/commonHttpFunctions.c ../commonSrc/argValidator.c ./main.c ../commonSrc/mime.c ../commonSrc/net.c

All :  fullMake clean init server-predefine-threads

init:
	mkdir build

fullMake:
	@echo Full make

server-predefine-threads: $(OBJECTS)
	@echo Server Predefine Threads
	gcc  $(CFLAGS) $(HEADERS) -o build/server-predefine-threads $(OBJECTS) $(LIBRARIES)

clean:
	@echo Clean compile
	rm -rf build
