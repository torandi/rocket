#include "connection.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "sha1.h"
#include "client.h"

int server_port=4711;
int local_port=4711;
std::string server_hostname="localhost";

/**
 * Stop all sockets
 */
void terminate_sockets() {
	if(server_sock!=0)
		close(server_sock);
	if(client_sock!=0)
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

socket_data * init_socket(int sock,bool secure) {
	socket_data * socket = (socket_data*) malloc(sizeof(socket_data));

	socket->socket = sock;
	socket->secure = secure; 
	socket->key = FIRST_KEY;
	return socket;
}

/* Sends data through socket */
void write_data(const socket_data * sock,const void * data,int len) {
	int n = send(sock->socket,data,len,MSG_DONTROUTE);
		
	if(n<0) {
		fprintf(stderr,"Failed to write to socket %i (%s).\n",sock->socket,(char*)data);
	}
}

void writeln(const socket_data * sock,const void * data,int len) {
	if(sock == NULL) 
		return;
		
#if VERBOSE >=10
	printf("<<%s\n",(char*) data); 
#endif
	if(!sock->secure) {
		char * senddata = (char*)malloc(++len);
		memcpy(senddata,data,len);
		senddata[len-1]=0xA; //newline
		write_data(sock,senddata,len); //Send the data
		free(senddata);
	} else {
		if(((char*)data)[len-1]==0)
			--len; //Remove nullchar
		write_secret(sock,data,len);
	}
}

void write_secret(const socket_data * sock, const void * data,int len) {
	if(len > PAYLOAD_SIZE) {
		write_secret(sock,data,0);
		write_secret(sock,(char*)data+PAYLOAD_SIZE,len-PAYLOAD_SIZE);
	} else {
		char * buffer =(char*) malloc(FRAME_SIZE);
		char hash[41];
		char * garbage;
		get_hash(hash,(char*)data,len);
		buffer[0]=len;
		memcpy(buffer+1,hash,HASH_SIZE);
		if(len == 0)
			len = PAYLOAD_SIZE;
		memcpy(buffer+(FRAME_SIZE-len), data, len);
		//Fill the rest with random data:
		for(garbage = buffer+1+HASH_SIZE;garbage < buffer + (FRAME_SIZE-len);++garbage) {
			*garbage = rand() % 256;
		}

		encrypt(buffer,sock->key,FRAME_SIZE);
		#if VERBOSE >= 11	
		printf("utdata: %i|%s\n",buffer[0],&buffer[1]);
		for(int i=0;i<FRAME_SIZE;++i) {
			printf("%i, ",buffer[i]);
		}
		printf("\n");
		#endif
		write_data(sock,buffer,FRAME_SIZE);
		free(buffer);	
	}
}

int read_data(const socket_data * sock,void * data,size_t len) {
	if(sock == NULL)
		return -1;
	int ret = read(sock->socket,data,len);
	return ret;
}

int read_sck_line(const socket_data * sock,char * data) {
	char buffer[FRAME_SIZE+1];
	char * payload;
	char * hash;
	char size;
	int n = 0;
	bool cont=true;
	buffer[FRAME_SIZE]=0;

	if(sock == NULL) {
		data[0] = 0;
		return -1;
	}

	while(cont) {
		int r = read_data(sock,buffer,FRAME_SIZE);
		if(r <= 0) {
			#if VERBOSE>=2
				printf("Disconnected in read_sck_line\n");
			#endif
			data[0] = 0;
			return -1;
		}
		decrypt(buffer,sock->key,FRAME_SIZE);
		#if VERBOSE >= 11	
			printf("indata: %i|%s\n",buffer[0],&buffer[1]);
		#endif
		cont = (buffer[0] == 0);
		if (cont) {
			size = PAYLOAD_SIZE;
		} else {
			size = buffer[0];
		}
		payload = buffer + (FRAME_SIZE - size);
		hash = &buffer[1];
		if(cmp_hash(hash,payload,size)) {
			memcpy(data + n,payload,size);
			n += size;
		} else {
		#if VERBOSE >=2
				fprintf(stderr,"Incorrect hash for frame. Payload: %s. Data:\n",payload);
		#endif
		#if VERBOSE >= 11
				for(int i=0;i<FRAME_SIZE;++i) {
					printf("%i, ",buffer[i]);
				}
		#endif
			data[0]=0;
			return 0;
		}
	}
	data[n++]=0x0;
	return n;
}

void decrypt(char * data, char * key, int len) {
	data[len+1]=0;
	for(int n=0;n<len;++n) {
		data[n] = data[n] ^ key[0];
	}
}

void encrypt(char * data, char * key, int len) {
	decrypt(data,key,len);
}

void get_hash(char * hexstring,char * str, int len) {
	unsigned char hash[20];
	sha1::calc(str,len,hash);
	sha1::toHexString(hash, hexstring);
}

bool cmp_hash(char hash[HASH_SIZE],char * str,int len) {
	char hexstring[41];
	get_hash(hexstring,str,len);
	return (strncmp(hash,hexstring,HASH_SIZE) == 0);
}
