CURRFILE=$(shell cd ..; pwd)
HEADERS=-I$(CURRFILE)/headers/
CFLAGS=-g -Wall
LIBRARIES=-lm -pthread -lrt

OBJECTS=../commonSrc/argValidator.c ./main.c ../commonSrc/mime.c ../commonSrc/net.c

All :  fullMake clean init serverS

init:
	mkdir build

fullMake:
	@echo Full make

serverS: $(OBJECTS)
	@echo Server S
	gcc  $(CFLAGS) $(HEADERS) -o build/serverS $(OBJECTS) $(LIBRARIES)

clean:
	@echo Clean compile
	rm -rf build
