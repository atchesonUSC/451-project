# *****************************************************
# Variables to control Makefile operation
 
CC = gcc
CFLAGS_ALL = -O3 -Wall -g -lpthread -fopenmp
CFLAGS_NONE = -O3 -Wall -g
CFLAGS_OPENMP = -O3 -Wall -g -fopenmp
CFLAGS_THREADS = -O3 -Wall -g -lpthread
 
# ****************************************************
# Targets needed to bring the executable up to date
 
main: main.o tree.o barrier.o bmp2Map.o
	$(CC) $(CFLAGS_ALL) -o main main.o tree.o barrier.o bmp2Map.o
 
# The main.o target can be written more simply
 
main.o: main.cpp tree.h barrier.h bmp2Map.h bitmap_image.hpp
	$(CC) $(CFLAGS_ALL) -c main.cpp
 
tree.o: tree.cpp tree.h
	$(CC) $(CFLAGS_ALL) -c tree.cpp -o tree.o
 
barrier.o: barrier.cpp barrier.h
	$(CC) $(CFLAGS_THREADS) -c barrier.cpp -o barrier.o

bmp2Map.o: bmp2Map.cpp bmp2Map.h bitmap_image.hpp
	$(CC) $(CFLAGS_NONE) -c bmp2Map.cpp -o bmp2Map.o
