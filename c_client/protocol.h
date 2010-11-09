/** protcol contains all the server-client protocol */

#define VERSION "0.1"

#define PROT_AUTH "auth"
#define PROT_VERSION "cversion"
#define PROT_MODE_OK "mode ok"

#define PROT_GFX_CLEAR "clear"
#define PROT_GFX_UPDATE "update"
#define PROT_GFX_SHIP "ship"

#define PROT_GFX_ATTR_SHOOT "shoot"
#define PROT_GFX_ATTR_BOOST "boost"

#define PROT_SEND_MODE_DISPLAY "mode display\n"
#define PROT_SEND_MODE_BOT "mode bot\n"

/* Calculates the authkey from an authcode */
int protocol_authorize(const char * n);
