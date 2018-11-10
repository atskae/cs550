CC = gcc
CFLAGS = -Wall -g
all: producer consumer

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

producer: producer_numbers.o
	$(CC) producer_numbers.o -o producer

consumer: consumer_numbers.o
	$(CC) consumer_numbers.o -o consumer

clean:
	rm *.o producer consumer
