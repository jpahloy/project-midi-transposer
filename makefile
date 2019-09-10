CC = gcc
CFLAGS = -g


all : transpose-midi

clean:
	rm -f *.o transpose-midi *~ \#*\#

transpose-midi: main.c midi.o transposer.o
	$(CC) $(CFLAGS) main.c midi.o transposer.o -o transpose-midi

midi.o: midi.h

transposer.o: transposer.h

