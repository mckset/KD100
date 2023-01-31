CC = gcc
FLAGS = -lusb-1.0

install:
	${CC} ${FLAGS} KD100.c -o KD100

clean:
	rm -f KD100.c
