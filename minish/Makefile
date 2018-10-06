CC=gcc
CFLAGS= -g -Wall

minish: main.o
	$(CC) $(CFLAGS) main.o -o minish

main.o: main.c
	$(CC) $(CFLAGS) -c main.c
clean:
	rm *.o minish
