CURRFILE=$(shell cd ..; pwd)
HEADERS=-I$(CURRFILE)/headers/
CFLAGS=-g -Wall
LIBRARIES=-lm -pthread -lrt

OBJECTS=../commonSrc/argValidator.c ./main.c ../commonSrc/mime.c ../commonSrc/net.c

All :  fullMake clean init serverPT

init:
	mkdir build

fullMake:
	@echo Full make

serverPT: $(OBJECTS)
	@echo Server PT
	gcc  $(CFLAGS) $(HEADERS) -o build/serverPT $(OBJECTS) $(LIBRARIES)

clean:
	@echo Clean compile
	rm -rf build
