CC = gcc
CFLAGS = -Wall -g
all: producer consumer

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

producer: producer.o
	$(CC) producer.o -o producer

consumer: consumer.o
	$(CC) consumer.o -o consumer

clean:
	rm *.o producer consumer
