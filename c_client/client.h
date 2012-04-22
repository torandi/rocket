#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "connection.h"
#include "config.h"
#include "score.h"
#include <vector>

	//Verbose level: 0: nothing, 1: something 2-9: some more 11: All + all input/output from socket + encryption debug
	/*
	1: Server sync, server init info, frame dropping occurances
	2: Socket ready info, client disconnect info
	3: Score updates
	5: Information about gfx attributes
	6: Info about buffer memory
	7: Client forward output
	9: Frame dropping debug info
	10: Socket input/output
	11: Encryption debug
	 */
	#define VERBOSE 3


	#define CLIENT_VERSION "0.2 BETA"

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
