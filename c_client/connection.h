#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <stddef.h>
#include <string>

//#define SERVER_PORT 4711 //Port on botserver for bots
//#define CLIENT_PORT 4710 //Port that bots will connect to localy

//#define SERVER_HOSTNAME "192.168.0.5"
//#define SERVER_HOSTNAME "localhost"
//#define SERVER_HOSTNAME "10.0.0.114"
//#define SERVER_HOSTNAME "10.0.0.2"

#define FRAME_SIZE 32
#define HASH_SIZE 4
#define PAYLOAD_SIZE 27

#define FIRST_KEY "5"

typedef struct socket_data {
	int socket;
	bool secure;
	char * key;
} socket_data;

//These two methods writes to the socket
void write_data(const socket_data * sock,const void * data,int len);
void writeln(const socket_data * sock,const void * data,int len);


void write_secret(const socket_data * sock, const void * data,int len);

//reads len bytes from socket to data. Returns number of bytes read. Not encrypted
int read_data(const socket_data * sock,void * data,size_t len);

//reads a line from socket to data. Returns number of bytes read. Encrypted
int read_sck_line(const socket_data * sock,char * data);

void decrypt(char * data, char * key, int len);
void encrypt(char * data, char * key, int len);
bool cmp_hash(char hash[HASH_SIZE],char * str,int len);
void get_hash(char hash[HASH_SIZE],char * str,int len);

/** 
 * Initialises a connection (and creates a socket struct)
 */
socket_data * init_socket(int sock,bool secure);

//Close and deallocate a socket
void close_socket(socket_data ** sock);

//Stops all sockets
void terminate_sockets();

extern int server_port,local_port;
extern std::string server_hostname;
extern int server_sock; //Socket listening for new connections
extern socket_data * client_sock; //Socket connected to server
#endif
