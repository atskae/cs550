CC=gcc
CFLAGS=-Wall

minish: main.o
	$(CC) $(CFLAGS) main.o -o minish

main.o: main.c
	$(CC) -c main.c
clean:
	rm *.o minish
