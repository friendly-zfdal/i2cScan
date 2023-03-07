all: main
main: main.o i2c.o
	g++ -std=c++17 main.o i2c.o -o i2cScan
main.o: main.cpp i2c.h
	g++ -c -std=c++17 main.cpp
i2c.o: i2c.c i2c.h
	gcc -c i2c.c
clean:
	rm -f *.o
