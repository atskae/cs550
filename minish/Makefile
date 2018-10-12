CC = gcc
CFLAGS = -g -Wall
DEPS = minish.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

minish: main.o minish.o
	$(CC) main.o minish.o -o minish

clean:
	rm *.o minish
