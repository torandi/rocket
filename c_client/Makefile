LDFLAGS=-pthread -lSDL -lSDL_gfx -lSDL_ttf -lSDL_image
CFLAGS = -g -Wall

all: client

client: protocol.o client.o gfx.o connection.o

client.o: client.c connection.h gfx.h 

gfx.o: gfx.c gfx.h
	cc ${LDFLAGS} -c -o gfx.o gfx.c

connection.o: connection.c connection.h
	cc ${LDFLAGS} -c -o connection.o connection.c

clean:
	rm protocol.o client.o gfx.o connection.o client 
