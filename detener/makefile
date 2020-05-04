CURRFILE=$(shell cd ..; pwd)
HEADERS=-I$(CURRFILE)/headers/
CFLAGS=-g -Wall
LIBRARIES=-lm -pthread -lrt

OBJECTS=../commonSrc/argValidator.c ./detener.c ../commonSrc/mime.c ../commonSrc/net.c

All :  fullMake clean init detener

init:
	mkdir build

fullMake:
	@echo Full make

detener: $(OBJECTS)
	@echo Detener
	gcc  $(CFLAGS) $(HEADERS) -o build/detener $(OBJECTS) $(LIBRARIES)

clean:
	@echo Clean compile
	rm -rf build