# *****************************************************
# Variables to control Makefile operation
 
CC = g++
CFLAGS_NO_THREADS = -O3 -Wall -g -fpermissive
CFLAGS_THREADS = -O3 -Wall -g -fpermissive -lpthread
 
# ****************************************************
# Targets needed to bring the executable up to date
main: main.cpp tree.h barrier.h bmp2Map.h bitmap_image.hpp
	$(CC) $(CFLAGS_THREADS) main.cpp tree.o barrier.o bmp2Map.o -o main

main2: main.cpp tree.cpp barrier.cpp bmp2Map.cpp bitmap_image.hpp
	$(CC) $(CFLAGS_THREADS) main.cpp tree.cpp barrier.cpp bmp2Map.cpp bitmap_image.hpp -o main

tree.o: tree.cpp tree.h
	$(CC) $(CFLAGS_THREADS) -c tree.cpp -o tree.o
 
barrier.o: barrier.cpp barrier.h
	$(CC) $(CFLAGS_THREADS) -c barrier.cpp -o barrier.o

bmp2Map.o: bmp2Map.cpp bmp2Map.h bitmap_image.o
	$(CC) $(CFLAGS_NO_THREADS) -c bmp2Map.cpp bitmap_image.o -o bmp2Map.o

bitmap_image.o: bitmap_image.hpp
	$(CC) $(CFLAGS_NO_THREADS) -c bitmap_image.hpp -o bitmap_image.o
