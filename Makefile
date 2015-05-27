CC=gcc
CFLAGS=-g -Wall -D_GNU_SOURCE
EXEC=io_bench
LDFLAGS=

all: $(EXEC)
			
io_bench: io_bench.o
			$(CC) -o io_bench io_bench.o $(LDFLAGS)
		
io_bench.o: io_bench.c io_bench.h
			$(CC) -c io_bench.c $(CFLAGS)

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC) a.out
