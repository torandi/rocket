#include "connection.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

/**
 * Stop all sockets
 */
void terminate_sockets() {
	if(server_sock!=0)
		close(server_sock);
	close_socket(&client_sock);
}

/**
 * Stop a single socket
 */
void close_socket(socket_data ** sock) {
	close((*sock)->socket);
	free(*sock);
	*sock=0;
}

socket_data * init_socket(int sock,bool ssl) {
	socket_data * socket = (socket_data*) malloc(sizeof(socket_data));

	socket->socket=sock;
	socket->ssl=ssl;
	return socket;
}

/* Sends data through socket */
void write_data(const socket_data * sock,const void * data,int len) {
	int n = send(sock->socket,data,len,MSG_DONTROUTE);

	if(n<0) {
		fprintf(stderr,"Failed to write to socket %i (%s).\n",sock->socket,(char*)data);
	}
}

/* Sends data through socket with newline*/
void writeln(const socket_data * sock,const void * data,int len) {
	char * senddata = (char*)malloc(len+1);
	memcpy(senddata,data,len-1);
	senddata[len-1]=0xA; //newline
	senddata[len]=0; //Nullchar
#if VERBOSE >=10
	printf("<<%s\n",senddata); 
#endif
	write_data(sock,senddata,len); //Send the data
	free(senddata);	
}

int read_data(const socket_data * sock,void * data,size_t len) {
	int ret = read(sock->socket,data,len);
	return ret;
}
