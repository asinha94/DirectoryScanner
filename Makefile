CC=gcc

scan: scan.o
	$(CC) -o scan scan.o

.PHONY: clean

clean:
	rm -rf *~ *.o test a.out scan core
