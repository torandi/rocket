#ifndef _CLIENT_H_
#define _CLIENT_H_
	typedef enum {
		INIT=0,
		CONNECTED,
		VERSION_OK,
		AUTHORIZED,
		READY
	} server_state_t;

	#define MODE_BOT 1
	#define MODE_GFX 2

	typedef enum {
		AUTH_UNINITIALIZED,
		AUTH_REQUEST_SENT,
		AUTH_CODE_SENT,
		AUTH_DONE
	} auth_stage_t;


	//latversion av bool
	typedef unsigned char bool;
	#define true 1
	#define false 0



#endif
