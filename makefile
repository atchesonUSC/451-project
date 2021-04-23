# *****************************************************
# Variables to control Makefile operation
 
CC = g++
CFLAGS = -O3 -Wall -g -lpthread
 
# ****************************************************
# Targets needed to bring the executable up to date
main: main.cpp tree.h barrier.h bmp2Map.h bitmap_image.hpp
	$(CC) $(CFLAGS) main.cpp tree.o barrier.o bmp2Map.o -o main

main2: main.cpp tree.cpp barrier.cpp bmp2Map.cpp bitmap_image.hpp
	$(CC) $(CFLAGS) main.cpp tree.cpp barrier.cpp bmp2Map.cpp bitmap_image.hpp -o main

tree.o: tree.cpp tree.h
	$(CC) $(CFLAGS) -c tree.cpp -o tree.o
 
barrier.o: barrier.cpp barrier.h
	$(CC) $(CFLAGS) -c barrier.cpp -o barrier.o

bmp2Map.o: bmp2Map.cpp bmp2Map.h bitmap_image.o
	$(CC) $(CFLAGS) -c bmp2Map.cpp bitmap_image.o -o bmp2Map.o

bitmap_image.o: bitmap_image.hpp
	$(CC) $(CFLAGS) -c bitmap_image.hpp -o bitmap_image.o
