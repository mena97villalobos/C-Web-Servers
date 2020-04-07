CURRFILE=$(shell cd ..; pwd)
HEADERS=-I$(CURRFILE)/headers/
CFLAGS=-g -Wall
LIBRARIES=-lm -pthread -lrt

OBJECTS=../commonSrc/argValidator.c ./main.c ../commonSrc/mime.c ../commonSrc/net.c

All :  fullMake clean init serverT

init:
	mkdir build

fullMake:
	@echo Full make

serverT: $(OBJECTS)
	@echo Server T
	gcc  $(CFLAGS) $(HEADERS) -o build/serverT $(OBJECTS) $(LIBRARIES)

clean:
	@echo Clean compile
	rm -rf build