# *****************************************************
# Variables to control Makefile operation
 
CC = g++
CFLAGS = -O3 -lpthread
 
# ****************************************************
# Targets needed to bring the executable up to date

# Link command:
all: main.o tree.o barrier.o bmp2Map.o
	$(CC) $(CFLAGS) main.o tree.o barrier.o bmp2Map.o -o a.out

# compilation commands:
main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp -o main.o

tree.o: tree.cpp
	$(CC) $(CFLAGS) -c tree.cpp -o tree.o

barrier.o: barrier.cpp
	$(CC) $(CFLAGS) -c barrier.cpp -o barrier.o

bmp2Map.o: bmp2Map.cpp
	$(CC) $(CFLAGS) -c bmp2Map.cpp -o bmp2Map.o

# Clean object and executable files
clean:
	rm a.out *.o
