#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "connection.h"
#include "config.h"
#include "score.h"
#include <vector>

	#define CONFIG_FILE "rocket.conf"

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

	struct thread_data {
		int mode;
		socket_data * csock;
		socket_data * ssock;
	};
	
	extern Config config;
	extern std::vector<score_t> highscore;
#endif
