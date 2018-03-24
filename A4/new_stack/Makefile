CC = gcc

all: sharedstack.o sharedlib.a sample1.o sample2.o sample1 sample2

sample1: sample1.o sharedlib.a
	$(CC) -o sample1 sample1.o sharedlib.a

sample2: sample2.o sharedlib.a
	$(CC) -o sample2 sample2.o sharedlib.a

sharedstack.o:
	$(CC) -c sharedstack.c

sharedlib.a: sharedstack.o
	ar rs sharedlib.a sharedstack.o

sample1.o:
	$(CC) -I . -c sample1.c

sample2.o:
	$(CC) -I . -c sample2.c

clean:	
	rm *.o sample1 sample2 *.a