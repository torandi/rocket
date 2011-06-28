LDFLAGS=-pthread -lSDL -lSDL_image -lGL `pkg-config --libs ftgl`
CFLAGS = -g -Wall `pkg-config --cflags ftgl`

all: rocket
rocket: protocol.o client.o gfx.o connection.o config.o sha1.o
	g++ ${LDFLAGS} ${CFLAGS}  -o rocket protocol.o client.o gfx.o connection.o config.o sha1.o

client.o: client.cpp connection.h gfx.h score.h
	g++ ${LDFLAGS} ${CFLAGS} -c -o client.o client.cpp

gfx.o: gfx.cpp gfx.h score.h client.h
	g++ ${LDFLAGS} ${CFLAGS} -c -o gfx.o gfx.cpp

connection.o: connection.cpp connection.h client.h
	g++ ${LDFLAGS} ${CFLAGS} -c -o connection.o connection.cpp

protocol.o: protocol.cpp protocol.h client.h
	g++ ${LDFLAGS} ${CFLAGS} -c -o protocol.o protocol.cpp

config.o: config.cpp config.h client.h
	g++ ${LDFLAGS} ${CFLAGS} -c -o config.o config.cpp

sha1.o: sha1.h sha1.cpp
	g++ ${LDFLAGS} ${CFLAGS} -c -o sha1.o sha1.cpp

clean:
	rm protocol.o client.o gfx.o connection.o config.o rocket

install: rocket
	./install 

gfx: gfx.cpp
	g++ ${LDFLAGS} ${CFLAGS} -o gfx gfx.cpp

deb:
	dpkg-buildpackage -b
