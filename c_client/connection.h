#ifndef CONNECTION_H_
#define CONNECTION_H_

#include "common.h"
//#include "ssl.h"

#define SERVER_GFX_PORT 4711 //Port on botserver for gfx
#define SERVER_BOT_PORT 4711 //Port on botserver for bots
#define CLIENT_PORT 4710 //Port that bots will connect to localy

//#define SERVER_HOSTNAME "192.168.0.5"
#define SERVER_HOSTNAME "localhost"
//#define SERVER_HOSTNAME "10.0.0.104"

typedef struct socket_data {
	int socket;
	bool ssl;
} socket_data;

//These two methods writes to the socket
void write_data(const socket_data * sock,const void * data,int len);
void writeln(const socket_data * sock,const void * data,int len);

//reads len bytes from socket to data. Returns number of bytes read
int read_data(const socket_data * sock,void * data,int len);

/** 
 * Initialises a connection (and creates a socket struct)
 * sock should be a connected socket if ssl=1
 */
socket_data * init_socket(int sock,bool ssl);

//Close and deallocate a socket
void close_socket(socket_data * sock);

//Closes the sockets
void close_sockets();

int server_sock; //Socket listening for new connections
socket_data * client_sock; //Socket connected to server

#endif
