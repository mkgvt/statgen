CC = gcc

CFLAGS = -g -O3
LDFLAGS = -lm

all: statgen

statgen: statgen.o #getopt.o
	gcc $(CFLAGS) -o statgen statgen.o $(LDFLAGS)

statgen.o: statgen.c
	gcc $(CFLAGS) -c $<

clean:
	rm -f *.o

distclean: clean
	rm -f *~ statgen

