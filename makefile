# *****************************************************
# Variables to control Makefile operation
 
CC = g++
CFLAGS_ALL = -O3 -Wall -g -lpthread -fopenmp
CFLAGS_NONE = -O3 -Wall -g
CFLAGS_OPENMP = -O3 -Wall -g -fopenmp
CFLAGS_THREADS = -O3 -Wall -g -lpthread
 
# ****************************************************
# Targets needed to bring the executable up to date
main: main.cpp tree.h barrier.h bmp2Map.h bitmap_image.hpp
	$(CC) $(CFLAGS_ALL) main.cpp tree.o barrier.o bmp2Map.o -o main

main2: main.cpp tree.cpp barrier.cpp bmp2Map.cpp bitmap_image.hpp
	$(CC) $(CFLAGS_ALL) main.cpp tree.cpp barrier.cpp bmp2Map.cpp bitmap_image.hpp -o main

tree.o: tree.cpp tree.h
	$(CC) $(CFLAGS_ALL) -c tree.cpp -o tree.o
 
barrier.o: barrier.cpp barrier.h
	$(CC) $(CFLAGS_THREADS) -c barrier.cpp -o barrier.o

bmp2Map.o: bmp2Map.cpp bmp2Map.h bitmap_image.o
	$(CC) $(CFLAGS_NONE) -c bmp2Map.cpp bitmap_image.o -o bmp2Map.o

bitmap_image.o: bitmap_image.hpp
	$(CC) $(CFLAGS_NONE) -c bitmap_image.hpp -o bitmap_image.o