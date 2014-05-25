all: tmatch

tmatch: tmatch.c
	gcc -Wall -o tmatch tmatch.c -lrt -std=c99 -lpthread

clean:
	rm -fr *~ tmatch
