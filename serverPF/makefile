CURRFILE=$(shell cd ..; pwd)
HEADERS=-I$(CURRFILE)/headers/
CFLAGS=-g -Wall
LIBRARIES=-lm -lrt -lpthread

OBJECTS=../commonSrc/commonHttpFunctions.c ../commonSrc/argValidator.c ./main.c ../commonSrc/mime.c ../commonSrc/net.c

All :  fullMake clean init serverPredefineForks

init:
	mkdir build

fullMake:
	@echo Full make

serverPredefineForks: $(OBJECTS)
	@echo Server Predefine Forks
	gcc  $(CFLAGS) $(HEADERS) -o build/serverPredefineForks $(OBJECTS) $(LIBRARIES)

clean:
	@echo Clean compile
	rm -rf build