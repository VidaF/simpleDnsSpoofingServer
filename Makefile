CC=gcc
CFLAGS=-I./include
DEPS = include/dnsSpecHeader.h include/dnsSpoofing.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

dnsSpoofing: dnsSpoofing.o
	$(CC) -o dnsSpoofing dnsSpoofing.o

dnsTest: test/dnsTest.o
	$(CC) -o test/dnsTest test/dnsTest.o

all: dnsSpoofing dnsTest

clean:
	rm -f *.o dnsSpoofing
	rm -f test/*.o test/dnsTest
