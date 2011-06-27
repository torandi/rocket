#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <limits.h>
#include <vector>
#include <math.h>
#include <exception>
#include <algorithm>

#include "connection.h"
#include "client.h"
#include "gfx.h"
#include "protocol.h"
#include "ship.h"
#include "config.h"

#define CMP_BUFFER(s) strncmp(buffer,s,strlen(s))==0

//If more than this number of frames are dropped or negative, resync offset
#define MAX_WRONG_FRAMES 10

#define BUFFER_SIZE 1024

bool nox=false;

socket_data * init_server_connection(struct thread_data * data);
void * init_server_communication(void * data);
void read_server(struct thread_data *td);
void * read_client(void *data);
void *new_client_thread(void *data);
void * init_server(void * data);
void * gfx_hndl(void * data);
void * start_gfx(void * data);
std::vector<score_t>::iterator find_score(int id);
double get_time();
void calculate_interpolated_position(ship_t &ship, double delay);
void check_bounds(ship_t &ship);

pthread_t update_gfx_t;
pthread_t event_t; //Thread for glut window
pthread_t server_t; //Thread for handling clients
pthread_t client_t; //Thread for communicating with server
pthread_mutex_t frame_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t highscore_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t gfx_draw_lock=PTHREAD_MUTEX_INITIALIZER;
const double max_frame_age=0.5; //The max age of a frame in seconds

int server_sock; //Socket listening for new connections
socket_data * client_sock; //Socket connected to server

std::vector<ship_t> ships;
double last_frame;

Config config;
std::vector<score_t> highscore;

int main(int argc,char *argv[])
{
	printf("RoBot Sockets v. %s\n",CLIENT_VERSION);
	//Try some config files:
	if(!config.open("rocket.conf")) {
		if(!config.open(std::string(getenv("HOME"))+"/.rocket.conf")) {
			if(!config.open("/usr/share/rocket/rocket.conf")) {
				fprintf(stderr,"No rocket.conf found\n");
				exit(-1);
			}
		}
	}
	try {
		server_hostname=config["server_hostname"];
		server_port=atoi(config["server_port"].c_str());
		local_port=atoi(config["local_port"].c_str());
	} catch (std::exception &e) {
		printf("Failed to read all data from config\n");
	}

	for(int i=0;i<argc;++i) {
		if(strcmp(argv[i],"-nox")==0)  {
			nox=true;
		}
		if(strcmp(argv[i],"-s")==0 || strcmp(argv[i],"--server")==0) {
			if(argc>i+1) {
				server_hostname=std::string(argv[i+1]);
			} else {
				fprintf(stderr,"Missing parameter to argument %s\n",argv[i]);
			}
		}
		if(strcmp(argv[i],"-sp")==0 || strcmp(argv[i],"--server-port")==0) {
			if(argc>i+1) {
				server_port=atoi(argv[i+1]);
			} else {
				fprintf(stderr,"Missing parameter to argument %s\n",argv[i]);
			}
		}
		if(strcmp(argv[i],"-lp")==0 || strcmp(argv[i],"--local-port")==0) {
			if(argc>i+1) {
				local_port=atoi(argv[i+1]);
			} else {
				fprintf(stderr,"Missing parameter to argument %s\n",argv[i]);
			}
		}
		if(strcmp(argv[i],"-h")==0 || strcmp(argv[i],"--help")==0) {
			printf("Arguments:\n\t-nox: Turn off graphics\n\t -s, --server <server>  : Set server\n\t-sp,--server-port <port> : Set server port\n\t-lp,--local-port : Set which port to listen to\n\t-h,--help : Show this help text\n");
			exit(0);
		}
	}

	last_frame=get_time();
	if(!nox) {
		struct thread_data td;
		td.mode=MODE_GFX;
		td.ssock=init_server_connection(&td);
		td.csock=0;
		client_sock=td.ssock;

		if(td.ssock==0) 
			exit(1);


	#if VERBOSE>=1
		printf("GFX socket: %i\n",client_sock->socket);
	#endif
		void * data = malloc(sizeof(td));
		memcpy(data,&td,sizeof(td));
		pthread_create(&client_t,NULL,init_server_communication,data);
	} else {
		printf("Display disabled\n");
		client_sock=0;
		pthread_create(&server_t,NULL,init_server,NULL);
	}
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
		port=server_port;
	else if(td->mode==MODE_BOT) 
		port=server_port;
	else {
		fprintf(stderr,"Internal error: Unknown mode\n");
		exit(2);
	}

	printf("Connecting to server %s:%i\n",server_hostname.c_str(),port);

	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0) {
		fprintf(stderr,"Failed to initalize socket\n");
		exit(-1);
	}

	server = gethostbyname(server_hostname.c_str());
	if(server==NULL) {
		fprintf(stderr,"No such host %s\n",server_hostname.c_str());
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
	
	return init_socket(sockfd,true);

}


void * init_server_communication(void * data) {
	char buffer[50];
	int len;
	thread_data * td=(thread_data*) malloc(sizeof(struct thread_data));
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
	char * buffer = (char*) calloc(BUFFER_SIZE,sizeof(int)); //readbuffer
	bool run=true;
	auth_stage_t auth_stage=AUTH_UNINITIALIZED;
	bool mode_ok=false;
	bool active_frame=false;
	double server_time_offset; //The number of seconds we differ from the server

	std::vector<ship_t> frame;

	int ignored_frames=0; //count the number of ignored frames
	int num_neg_frames=0; //Number of frames with negative age

	pthread_t client_thread_t; //The client thread

	while(run) {

		int n = read_sck_line(td->ssock,buffer);
		if(!run || n<0)
			break;
		
#if VERBOSE >= 10
		printf(">>%s\n",buffer);
#endif

		double local_time=get_time();


		//cversion
		if(CMP_BUFFER(PROT_VERSION)) {
			if(strcmp(buffer+strlen(PROT_VERSION)+1,"yes")==0) {
#if VERBOSE >= 1
				printf("Version good\n");
#endif
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
#if VERBOSE >= 2
						printf("Time offset to server: %f s\n",server_time_offset);
#endif
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
#if VERBOSE >= 3
			printf("mode accepted (%i)\n",td->mode);
#endif
			mode_ok=true;
			//if gfx mode, start gfx
			if(td->mode==MODE_GFX) {
				int screen_width, screen_height;
				sscanf(buffer,PROT_MODE_OK_GFX,&screen_width,&screen_height);
				//init graphics
				init_gfx(screen_width,screen_height);
				pthread_create(&event_t,NULL,start_gfx,NULL);
				//Thread for handling sdl events
				pthread_create(&update_gfx_t,NULL,gfx_hndl,NULL);
				//Start server
				pthread_create(&server_t,NULL,init_server,NULL);
				continue;
			} else if(td->mode==MODE_BOT) {
				//Start thread for forwardning from client->botserver
				pthread_create(&client_thread_t,NULL,read_client,td);
				//Tell client that we are ready to forward data to the server
#if VERBOSE >= 2
				printf("Bot socket ready\n");
#endif
				writeln(td->csock,PROT_BOT_READY,sizeof(PROT_BOT_READY));
				continue;
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
#if VERBOSE >= 9
					printf("Frame age: %lf s\n",age);
#endif
					active_frame=true;
					frame.clear();
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
					active_frame=false;
#if VERBOSE >= 1 
					printf("Dropping frame! %lf s old.\n",age);
#endif
					if(++ignored_frames>MAX_WRONG_FRAMES) {
						//Resync!
						age=local_time-server_time;
						ignored_frames=0;
						server_time_offset=age;
#if VERBOSE >= 1
						printf("Resynced offset to server. New offset: %lf s\n",age);
#endif
					}
					continue;
				}
			} else if(CMP_BUFFER(PROT_GFX_FRAME_STOP)) {
				if(active_frame) {
					pthread_mutex_lock(&frame_lock);
					ships=frame;
					last_frame=local_time;
					pthread_mutex_unlock(&frame_lock);
				}
			} else if(CMP_BUFFER(PROT_GFX_SHIP)) {
				if(active_frame) {
					ship_t ship;
					void * attr_org;
					char * attr_str=(char*)malloc(128);
					attr_org=attr_str; //save a pointer to orginal allocated memory
					bzero(ship.attr,NUM_GFX_ATTR); //Make sure attr only contains false (0)
					
					int n=sscanf(buffer,PROT_GFX_SHIP_DATA,ship.nick,&ship.x,&ship.y,&ship.a,&ship.s,attr_str);
					if(n>=PROT_GFX_SHIP_DATA_MIN_ARGS) {
						if(n==PROT_GFX_SHIP_DATA_ATTR_ARGS) {
	#if VERBOSE >= 5
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
	#if VERBOSE >= 5
								printf("Found attr: %s\n",cur_attr);
	#endif
								if(strcmp(cur_attr,PROT_GFX_ATTR_SHOOT)==0) {
									ship.attr[GFX_ATTR_SHOOT]=true;
								} else if(strcmp(cur_attr,PROT_GFX_ATTR_BOOST)==0) {
									ship.attr[GFX_ATTR_BOOST]=true;
								} else if(strcmp(cur_attr,PROT_GFX_ATTR_SCAN)==0) {
									ship.attr[GFX_ATTR_SCAN]=true;
								} else {
#if VERBOSE >= 3
									fprintf(stderr,"Got unknown attribute %s\n",cur_attr);
#endif
								}
							}
						}
						frame.push_back(ship);
						free(attr_org);
					} else {
						fprintf(stderr,"Invalid data from gfx server (ship had %i args, expected %i)\n",n,PROT_GFX_SHIP_DATA_MIN_ARGS);
					}
				}
			} else if (CMP_BUFFER(PROT_SCORE)) {
				score_t score;
				int n=sscanf(buffer,PROT_SCORE_DATA,&score.id,score.nick,&score.score);
				if(n==PROT_SCORE_DATA_ARGS) {
					std::vector<score_t>::iterator s_it=find_score(score.id);
					pthread_mutex_lock(&highscore_lock);
					if(s_it!=highscore.end()) {
						strcpy(s_it->nick,score.nick);
						s_it->score=score.score;
					} else {
						highscore.push_back(score);
					}
					std::sort(highscore.begin(),highscore.end());
					pthread_mutex_unlock(&highscore_lock);
				} else {
					fprintf(stderr,"Invalid data from gfx server (score had %i args, expected %i)\n",n,PROT_SCORE_DATA_ARGS);
				}

			} else {
				fprintf(stderr,"Got unknown command: %s\n",buffer);
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
			writeln(td->csock,buffer,strlen(buffer));
		}
		/*
		 * End BOT mode
		 */

		

	}

#if VERBOSE>=1
	if(td->ssock!=0)
		printf("Disconnected from server (socket: %i)\n",td->ssock->socket);
	else
		printf("Disconnected from server (socket already unallocated)\n");
#endif
	free(buffer);

	if(td->ssock!=0) {
		if(td->ssock->socket==server_sock)
			server_sock=0;

		close_socket(&td->ssock);
	}

	if(td->mode==MODE_GFX && mode_ok) {
		pthread_join(client_thread_t,NULL);
		pthread_mutex_lock(&gfx_draw_lock);
		stop_gfx();
		exit(0);
	}


	if(td->csock!=0) {
		close_socket(&td->csock);
	}
}

/**
 * Handles communication from the player (and forwards to the robot server)
 */
void * read_client(void * data) {
	struct thread_data *td=(struct thread_data*)data;

	char * buffer = (char*)malloc(BUFFER_SIZE); //readbuffer
	char * next_read = buffer;
	char * next_newline = NULL;
	bool run=true;

	while(run) {

		int n = (next_read - buffer);
		next_newline = strchr(buffer,'\n');
		if(next_newline == NULL) {
			n+=read_data(td->csock,(void*)next_read,BUFFER_SIZE-n);
			printf("Read: %s\n",next_read);
			
			if(n<=0) {
				run=false;
				break;
			}
			//Find next newline
			next_newline = strchr(buffer,'\n');
		}

		if(CMP_BUFFER(PROT_CLOSE)) {
#if VERBOSE >= 2
			printf("Client closed connection\n");
#endif
			break;
		}
	
		if(next_newline == NULL) {
			if(n == BUFFER_SIZE) {
				run = false;
				break;
			} else {
				next_read = buffer + n;
				continue;
			}
		} else {
			*next_newline = 0; //For output
		}

		//Forward data
		int len = (next_newline-buffer);
		if(len > 0) {
#if VERBOSE >= 7
			printf("Forwarded:(%i) %s\n",len,buffer);
#endif
			writeln(td->ssock,buffer,len);
		}
		++len; //Now include newline in lenght
		if(len < n) {
			memcpy(buffer,next_newline+1,n-len);
			#if VERBOSE >= 6
				printf("Client forward: %i bytes left in buffer\n",n-len);
			#endif
			next_read = buffer + (n - len);
		} else {
			next_read = buffer;
		}
	}

#if VERBOSE >=1 
	printf("Disconnected from client (socket: %i)\n",td->csock->socket);
#endif
	if(td->ssock!=0) {
		writeln(td->ssock,PROT_CLOSE,sizeof(PROT_CLOSE));
		close_socket(&td->ssock);
	}
	if(td->csock!=0) {	
		close_socket(&td->csock);
	}
	free(buffer);
	return 0;
}

//inits server for bots
void * init_server(void * data)
{
     int newserver_sock;
	  unsigned int clilen;
     struct sockaddr_in serv_addr, cli_addr;
	  pthread_attr_t thread_attr;
	  int one=1;

     server_sock = socket(AF_INET, SOCK_STREAM, 0);
#if VERBOSE >= 1
	  printf("Server socket: %i\n",server_sock);
#endif

	  
	  setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,&one,sizeof(one));

#if VERBOSE >= 1
	  printf("Initializing server...\n");
#endif
	  

     if (server_sock < 0) {
        fprintf(stderr,"Error: Failed to open socket.\n");
			exit(1);
	  }
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(local_port);
     if (bind(server_sock, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) {
		  fprintf(stderr,"Error: Failed to bind socket to port %i\n",local_port);
		  exit(1);
	  }
 

	  //Set thread attributes
	  pthread_attr_init(&thread_attr);
	  pthread_attr_setdetachstate(&thread_attr,PTHREAD_CREATE_DETACHED);

	  printf("Ready for bots. Listening on port %i\n",local_port);

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
	td.csock=init_socket(sock,false);
	td.ssock=init_server_connection(&td);
	if(td.ssock==0) {
		fprintf(stderr,"Couldn't connect to botserver\n");
		return NULL;
	}
	data=malloc(sizeof(td));
	memcpy(data,&td,sizeof(td));

	init_server_communication(data); //start the connection to the server
	return NULL;
}

std::vector<score_t>::iterator find_score(int id) {
	std::vector<score_t>::iterator it;
	for(it=highscore.begin();it!=highscore.end();++it) {
		if(it->id==id) {
			return it;
		}
	}
	return highscore.end();
}

void update_gfx() {
	gfx_clear();
	pthread_mutex_lock(&frame_lock);
	std::vector<ship_t>::iterator it;
	for(it=ships.begin();it!=ships.end();++it) {
		calculate_interpolated_position(*it,get_time()-last_frame);
		draw_ship(*it);
	}
	pthread_mutex_unlock(&frame_lock);

	pthread_mutex_lock(&highscore_lock);
	draw_highscore();
	pthread_mutex_unlock(&highscore_lock);
	gfx_update();
}

/**
 * Updates ship->_x and _y with delay s gone since the last frame
 */
void calculate_interpolated_position(ship_t &ship, double delay) {
	ship._x=floor(ship.x+ship.s*delay*cos(ship.a)*GFX_SERVER_FPS);
	ship._y=floor(ship.y-ship.s*delay*sin(ship.a)*GFX_SERVER_FPS);
	check_bounds(ship);
}

void check_bounds(ship_t &ship) {
	if(ship._x < 0) { 
		ship._x = screen_width+ship._x; //x is already negtive!
	}
	if(ship._x > screen_width) {
		ship._x = ship._x - screen_width;
	}
	if(ship._y < 0) {
		ship._y = screen_height+ship._y; //y is already negtive!
	}
	if(ship._y > screen_height) { 
		ship._y  = ship._y - screen_height;
	}
}

/*
 * handles the sdl windows and closes it if necesary
 * Also handles the interpolation
 */
void * gfx_hndl(void * data) {
	while(1) {
		pthread_mutex_lock(&gfx_draw_lock);
		update_gfx();
		pthread_mutex_unlock(&gfx_draw_lock);
		usleep(1000/GFX_TARGET_FPS);
	}
	return NULL;
}

void * start_gfx(void * data) {
	start_event_loop();
	return NULL;
}

double get_time() {
	struct timeval t;
	gettimeofday(&t,0);
	return t.tv_sec + (t.tv_usec/1000000.0);
}
