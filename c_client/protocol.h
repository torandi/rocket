/** protcol contains all the server-client protocol */

#define VERSION "0.1"

#define PROT_AUTH "auth"
#define PROT_AUTH_OK "auth ok"
#define PROT_VERSION "cversion"
#define PROT_MODE_OK "mode ok"
#define PROT_MODE_OK_GFX "mode ok %i %i"
#define PROT_CLOSE "close"

#define PROT_GFX_FRAME_START "frame start"
#define PROT_GFX_FRAME_START_TIME "frame start %lf"
#define PROT_GFX_FRAME_STOP "frame stop"
#define PROT_GFX_SHIP "ship"
#define PROT_GFX_SHIP_DATA "ship %s %i %i %lf %f %s"

#define PROT_GFX_ATTR_SHOOT "shoot"
#define PROT_GFX_ATTR_BOOST "boost"
#define PROT_GFX_ATTR_SCAN "scan"

#define PROT_SEND_MODE_DISPLAY "mode display\n"
#define PROT_SEND_MODE_BOT "mode bot\n"

#define PROT_BOT_READY "ready"

/* Calculates the authkey from an authcode */
int protocol_authorize(const char * n);
