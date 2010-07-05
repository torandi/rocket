LDFLAGS=-pthread -lSDL -lSDL_gfx -lSDL_ttf

all: client

client: protocol.o client.o gfx.o

gfx.o: gfx.c
	cc ${LDFLAGS} -c -o gfx.o gfx.c

clean:
	rm protocol.o client.o gfx.o client
