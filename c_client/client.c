/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <torandi@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 */

//Verbose level: 0: nothing, 1: something 2-9: some more 10: All + all input/output from socket
#define VERBOSE 2

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <limits.h>

#include "connection.h"
#include "client.h"
#include "gfx.h"
#include "common.h"
#include "protocol.h"

#define CMP_BUFFER(s) strncmp(buffer,s,strlen(s))==0

//If more than this number of frames are dropped or negative, resync offset
#define MAX_WRONG_FRAMES 10

#define BUFFER_SIZE 2048
#define READ_SIZE 1024

socket_data * init_server_connection(struct thread_data * data);
void * init_server_communication(void * data);
void read_server(struct thread_data *td);
void * read_client(void *data);
void *new_client_thread(void *data);
void * init_server();
void * gfx_hndl();
double get_time();

pthread_t sdl_event_t;
pthread_t server_t; //Thread for handling clients
pthread_t client_t; //Thread for communicating with server
const double max_frame_age=0.5; //The max age of a frame in seconds


int main(int argc,char *argv[])
{
	//init_ssl();

	struct thread_data td;
	td.mode=MODE_GFX;
	td.ssock=init_server_connection(&td);
	td.csock=0;
	client_sock=td.ssock;

	if(td.ssock==0) 
		exit(1);

	printf("GFX socket: %i\n",client_sock->socket);
	void * data = malloc(sizeof(td));
	memcpy(data,&td,sizeof(td));
	pthread_create(&client_t,NULL,init_server_communication,data);
	//Make sure the sockets is closed on exit
	atexit(&terminate_sockets);

	//If the main thread exits using pthread_exit, the other will keep running
	pthread_exit(NULL);
}

/**
 * Initializes the connection to the robot server
 * @return The int for the socket that was created
 */
socket_data * init_server_connection(struct thread_data * td)
{
	int sockfd;
	struct sockaddr_in serv_addr;
	struct hostent * server;


	int port;
	if(td->mode==MODE_GFX)
		port=SERVER_GFX_PORT;
	else if(td->mode==MODE_BOT) 
		port=SERVER_BOT_PORT;
	else {
		fprintf(stderr,"Internal error: Unknown mode\n");
		exit(2);
	}

	printf("Connecting to server %s:%i\n",SERVER_HOSTNAME,port);

	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0) {
		fprintf(stderr,"Failed to initalize socket\n");
		exit(-1);
	}

	server = gethostbyname(SERVER_HOSTNAME);
	if(server==NULL) {
		fprintf(stderr,"No such host %s\n",SERVER_HOSTNAME);
		exit(-1);
	}

	//Set server addr and port in the socket
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
			      (char *)&serv_addr.sin_addr.s_addr,
					      server->h_length);

	serv_addr.sin_port = htons(port);

	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		fprintf(stderr,"Connection failed.\n");
		return 0;
	} else {
		printf("Connected to server\n");
	}
	
	return init_socket(sockfd,1);

}


void * init_server_communication(void * data) {
	char buffer[50];
	int len;
	struct thread_data * td=malloc(sizeof(struct thread_data));
	memcpy(td,data,sizeof(struct thread_data));
	free(data);

	strcpy(buffer,PROT_VERSION);
	len=strlen(buffer);

	buffer[len++]=0x20; //space
	strcpy(buffer+len,VERSION);

	writeln(td->ssock,buffer,strlen(buffer)+1);
	
	read_server(td);
	free(td);
	return 0;
}

/**
 * Handles communication with the robot server
 */
void read_server(struct thread_data *td) {
	char * buffer = calloc(BUFFER_SIZE,sizeof(int)); //readbuffer
	char * buffer2 = NULL; //readbuffer
	char * data=NULL; //data to parse
	char * next_newline=NULL;
	bool run=true;
	char * next_write=buffer; //next position in buffer to write to
	auth_stage_t auth_stage=AUTH_UNINITIALIZED;
	bool mode_ok=false;
	double server_time_offset; //The number of seconds we differ from the server

	int ignored_frames=0; //count the number of ignored frames
	int num_neg_frames=0; //Number of frames with negative age

	while(run) {
		int n;


		while(next_newline==NULL) {
			n=read_data(td->ssock,next_write,READ_SIZE);
			if(n<=0) {
				run=false;
				break;
			}
			next_newline=strchr(buffer,'\n');
			next_write+=n;
		}
		if(!run)
			break;
		
		data=buffer;
		//Set the char at next_newline to \0
		*next_newline=0;

#if VERBOSE >= 10
		printf(">>%s\n",data);
#endif

		double local_time=get_time();


		//cversion
		if(CMP_BUFFER(PROT_VERSION)) {
			if(strcmp(buffer+strlen(PROT_VERSION)+1,"yes")==0) {
				printf("Version good\n");
			} else if(strncmp(buffer+strlen(PROT_VERSION)+1,"ok",2)==0) {
				const char * msg=buffer+strlen(PROT_VERSION)+4;
				printf("Version ok\n%s\n",msg);
			} else if(strncmp(buffer+strlen(PROT_VERSION)+1,"no",2)==0) {
				const char * msg=buffer+strlen(PROT_VERSION)+4;
				printf("Invalid version\n%s\n",msg);
				run=false;
				break;
			} else {
				printf("Invalid version\n");
				run=false;
				break;
			}
			auth_stage=AUTH_REQUEST_SENT;
			writeln(td->ssock,PROT_AUTH,sizeof(PROT_AUTH));
		}
		if(CMP_BUFFER(PROT_AUTH)) {
			switch(auth_stage) {
				case AUTH_REQUEST_SENT:
					{
						char * auth_code;
						int auth_answer;
						auth_code=buffer+strlen(PROT_AUTH)+1;
						auth_answer=protocol_authorize(auth_code);
						sprintf(buffer,"%s %i",PROT_AUTH,auth_answer);
						auth_stage=AUTH_CODE_SENT;
						writeln(td->ssock,buffer,strlen(buffer)+1);
						break;
					}
				case AUTH_CODE_SENT:
					if(CMP_BUFFER(PROT_AUTH_OK)) {
						double server_time;
						sscanf(buffer,"auth ok %lf",&server_time);
						server_time_offset=local_time-server_time;
						printf("Time offset to server: %f s\n",server_time_offset);
						auth_stage=AUTH_DONE;
					} else {
						fprintf(stderr,"Authorisation failed\n");
						run=false;
					}
					break;
				default:
					fprintf(stderr,"Invalid answer from server in authorisation, disconnecting\n");
					run=false;
			}

			if(auth_stage==AUTH_DONE) {
				switch(td->mode) {
					case MODE_GFX:
						writeln(td->ssock,PROT_SEND_MODE_DISPLAY,sizeof(PROT_SEND_MODE_DISPLAY));
						break;
					case MODE_BOT:
						writeln(td->ssock,PROT_SEND_MODE_BOT,sizeof(PROT_SEND_MODE_BOT));
						break;
					default:
						printf("Unknown mode\n");
					
				}
			}
		} else if(CMP_BUFFER(PROT_MODE_OK)) {
			printf("mode accepted (%i)\n",td->mode);
			mode_ok=true;
			//if gfx mode, start sdl
			if(td->mode==MODE_GFX) {
				init_sdl();
				//Thread for handling sdl events
				pthread_create(&sdl_event_t,NULL,gfx_hndl,NULL);
				//Start server
				pthread_create(&server_t,NULL,init_server,NULL);
			} else if(td->mode==MODE_BOT) {
				//Start thread for forwardning from client->botserver
				pthread_t tmp_thread_t;
				pthread_create(&tmp_thread_t,NULL,read_client,td);
				//Tell client that we are ready to forward data to the server
				writeln(td->csock,"ready",sizeof("ready"));
				goto parsing_done; //jump to parsing done (to prevent forwarding "mode ok")
			}
		}

		/*
		 * Start GFX mode
		 */
		if(td->mode==MODE_GFX && mode_ok) {
			if(CMP_BUFFER(PROT_GFX_FRAME_START)) {
				double server_time;
				sscanf(buffer,PROT_GFX_FRAME_START_TIME,&server_time);
				double age=local_time-(server_time+server_time_offset);
				if(age<max_frame_age) {
#if VERBOSE >= 3
					printf("Frame age: %lf s\n",age);
#endif
					gfx_clear();
					ignored_frames=0;
					if(age<-0.1) { //Negative age
						if(++num_neg_frames>MAX_WRONG_FRAMES) {
							num_neg_frames=0;
							age=local_time-server_time;
							server_time_offset=age;
#if VERBOSE >= 1
							printf("Resynced offset to server (neg). New offset: %lf s\n",age);
#endif
						}
					} else {
						num_neg_frames=0;
					}
				} else {

#if VERBOSE >= 1 
					printf("Dropping frame! %lf s old.\n",age);
#endif
					//Drop data in buffer
					next_write=NULL;

					if(++ignored_frames>MAX_WRONG_FRAMES) {
						//Resync!
						age=local_time-server_time;
						ignored_frames=0;
						server_time_offset=age;
#if VERBOSE >= 1
						printf("Resynced offset to server. New offset: %lf s\n",age);
#endif
					}
					goto parsing_done;
				}
			} else if(CMP_BUFFER(PROT_GFX_FRAME_STOP)) {
				gfx_update();
			} else if(CMP_BUFFER(PROT_GFX_SHIP)) {
				char nick[32];
				bool attr[NUM_GFX_ATTR];
				int x, y, a;
				void * attr_org;
				char * attr_str=malloc(128);
				attr_org=attr_str; //save a pointer to orginal allocated memory
				bzero(attr,NUM_GFX_ATTR); //Make sure attr only contains false (0)
				
				int n=sscanf(buffer,PROT_GFX_SHIP_DATA,nick,&x,&y,&a,attr_str);
				if(n>=4) {
					if(n==5) {
#if VERBOSE >= 4
						printf("Attr: %s\n",attr_str);
#endif
						//Got attributes
						char cur_attr[32];
						int pos;
						while(strlen(attr_str)>0) {
							bzero(cur_attr,32);
							pos=0;
							while(*attr_str!=0x2C && *attr_str!=0) // 0x2C=','
								cur_attr[pos++]=*(attr_str++);
							if(*attr_str!=0)
								++attr_str;
#if VERBOSE >= 4
							printf("Found attr: %s\n",cur_attr);
#endif
							if(strcmp(cur_attr,PROT_GFX_ATTR_SHOOT)==0) {
								attr[GFX_ATTR_SHOOT]=true;
							} else if(strcmp(cur_attr,PROT_GFX_ATTR_BOOST)==0) {
								attr[GFX_ATTR_BOOST]=true;
							} else {
								fprintf(stderr,"Got unknown attribute %s\n",cur_attr);
							}
						}
					}
					draw_ship(nick,x,y,a,attr);
					free(attr_org);
				} else {
					fprintf(stderr,"Invalid data from gfx server\n");
				}
			}
		}

		/*
		 * End GFX mode
		 */

		/*
		 * Start BOT mode
		 */
		if(td->mode==MODE_BOT && mode_ok) {
			//Forward data
			writeln(td->csock,data,strlen(data)+1);
		}
		/*
		 * End BOT mode
		 */

		

parsing_done:
		//parsing done
		//increase next_newline since we only are intrested in the byte after now
		next_newline++;
		//If next_write>next_newline there is remaining data in the buffer
		if(next_write>next_newline) {
			//keep remaining data in buffer
			buffer2=calloc(BUFFER_SIZE,sizeof(int));
			memcpy(buffer2,next_newline,next_write-next_newline);
			free(buffer);
			buffer=buffer2;
			next_write=buffer2+(next_write-next_newline);
			next_newline=strchr(buffer,'\n'); //Find next newline in buffer (or NULL if cont to read)
			//printf("Moved around in buffer, is now: %s\n",buffer); 
		} else {
			free(buffer);
			buffer=calloc(1024,sizeof(int));
			next_write=buffer;
			next_newline=NULL;
		}
	}

	printf("Disconnected from server (socket: %i)\n",td->ssock->socket);
	free(buffer);

	if(td->ssock->socket==server_sock)
		server_sock=0;

	close_socket(td->ssock);


	if(td->csock!=0)
		close_socket(td->csock);
}

/**
 * Handles communication from the player (and forwards to the robot server)
 */
void * read_client(void * data) {
	struct thread_data *td=(struct thread_data*)data;

	char * buffer = calloc(1024,sizeof(int)); //readbuffer
	bool run=true;

	while(run) {
		int n;

		n=read_data(td->csock,buffer,1024);
		if(n<=0) {
			run=false;
			break;
		}
		//Forward data
		write_data(td->ssock,buffer,n);
	}

	printf("Disconnected from server (socket: %i)\n",td->ssock->socket);
	write_data(td->ssock,"close",sizeof("close"));
	close_socket(td->csock);
	free(buffer);
	return 0;
}

//inits server for bots
void * init_server()
{
     int newserver_sock;
	  unsigned int clilen;
     struct sockaddr_in serv_addr, cli_addr;
	  pthread_attr_t thread_attr;
	  int one=1;

     server_sock = socket(AF_INET, SOCK_STREAM, 0);
	  printf("Server socket: %i\n",server_sock);

	  
	  setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,&one,sizeof(one));

	  printf("Initializing server...\n");
	  

     if (server_sock < 0) {
        fprintf(stderr,"Error: Failed to open socket.\n");
			exit(1);
	  }
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(CLIENT_PORT);
     if (bind(server_sock, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) {
		  fprintf(stderr,"Error: Failed to bind socket to port %i\n",CLIENT_PORT);
		  exit(1);
	  }
 

	  //Set thread attributes
	  pthread_attr_init(&thread_attr);
	  pthread_attr_setdetachstate(&thread_attr,PTHREAD_CREATE_DETACHED);

	  printf("Server is ready\n");

	  while(1) {
		  listen(server_sock,5); //5 is the lenght of the queue for
		  								 // pending connections
		  clilen = sizeof(cli_addr);
		  newserver_sock = accept(server_sock, 
						  (struct sockaddr *) &cli_addr, 
						  &clilen);
		  if (newserver_sock < 0) {
				 fprintf(stderr,"Error: Failed to accept'\n");
				 continue;
		  }
		pthread_t tmp_client_t;
		void * data=malloc(sizeof(int));
		memcpy(data,&newserver_sock,sizeof(int));
		pthread_create(&tmp_client_t,&thread_attr,new_client_thread,data);
	}
}

void *new_client_thread(void *data) {
	int sock;
	memcpy(&sock,data,sizeof(int));
	free(data);

	struct thread_data td;
	td.mode=MODE_BOT;
	td.csock=init_socket(sock,0);
	td.ssock=init_server_connection(&td);
	if(td.ssock==0) {
		fprintf(stderr,"Couldn't connect to botserver\n");
		return NULL;
	}
	printf("Client socket: %i\n",client_sock->socket);
	data=malloc(sizeof(td));
	memcpy(data,&td,sizeof(td));
	init_server_communication(data); //start the connection to the server
	return NULL;
}

/*
 * handles the sdl windows and closes it if necesary
 */
void * gfx_hndl() {
	while(1) {
		if(hndl_sdl_events()!=0){
			exit(0);
		}
		usleep(100);
	}
}

double get_time() {
	struct timeval t;
	gettimeofday(&t,0);
	return t.tv_sec + (t.tv_usec/1000000.0);
}
