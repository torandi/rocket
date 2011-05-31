LDFLAGS=-pthread -lSDL -lSDL_gfx -lSDL_ttf -lSDL_image
CFLAGS = -g -Wall

all: rocket
rocket: protocol.o client.o gfx.o connection.o config.o
	g++ ${LDFLAGS} ${CFLAGS}  -o rocket protocol.o client.o gfx.o connection.o config.o

client.o: client.cpp connection.h gfx.h score.h
	g++ ${LDFLAGS} ${CFLAGS} -c -o client.o client.cpp

gfx.o: gfx.cpp gfx.h score.h
	g++ ${LDFLAGS} ${CFLAGS} -c -o gfx.o gfx.cpp

connection.o: connection.cpp connection.h
	g++ ${LDFLAGS} ${CFLAGS} -c -o connection.o connection.cpp

protocol.o: protocol.cpp protocol.h
	g++ ${LDFLAGS} ${CFLAGS} -c -o protocol.o protocol.cpp

config.o: config.cpp config.h
	g++ ${LDFLAGS} ${CFLAGS} -c -o config.o config.cpp

clean:
	rm protocol.o client.o gfx.o connection.o config.o rocket

install: rocket
	./install 

deb:
	dpkg-buildpackage
