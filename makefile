# *****************************************************
# Variables to control Makefile operation
 
CC = g++
CFLAGS = -Wall -g -lpthread -fopenmp
 
# ****************************************************
# Targets needed to bring the executable up to date
 
main: main.o tree.o barrier.o bmp2Map.o
    $(CC) $(CFLAGS) -o main main.o tree.o barrier.o bmp2Map.o
 
# The main.o target can be written more simply
 
main.o: main.cpp tree.h barrier.h bmp2Map.h bitmap_image.hpp
    $(CC) $(CFLAGS) -c main.cpp
 
tree.o: tree.cpp tree.h
	$(CC) $(CFLAGS) -c tree.cpp -o tree.o
 
barrier.o: barrier.cpp barrier.h
	$(CC) $(CFLAGS) -c barrier.cpp -o barrier.o

bmp2Map.o: bmp2Map.cpp bmp2Map.h bitmap_image.hpp
	$(CC) $(CFLAGS) -c bmp2Map.cpp -o bmp2Map.o
