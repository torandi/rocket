LDFLAGS=-pthread -lSDL -lSDL_gfx -lSDL_ttf -lSDL_image -lSDL_draw
CFLAGS = -g -Wall

all: client

client: protocol.o client.o gfx.o connection.o
	g++ ${LDFLAGS} ${CFLAGS} -o client protocol.o client.o gfx.o connection.o

client.o: client.cpp connection.h gfx.h 
	g++ ${LDFLAGS} ${CFLAGS} -c -o client.o client.cpp

gfx.o: gfx.cpp gfx.h
	g++ ${LDFLAGS} ${CFLAGS} -c -o gfx.o gfx.cpp

connection.o: connection.cpp connection.h
	g++ ${LDFLAGS} ${CFLAGS} -c -o connection.o connection.cpp

protocol.o: protocol.cpp protocol.h
	g++ ${LDFLAGS} ${CFLAGS} -c -o protocol.o protocol.cpp

clean:
	rm protocol.o client.o gfx.o connection.o client 
