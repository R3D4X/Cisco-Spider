#include <libssh2.h>

#define HAVE_SYS_SOCKET_H
#define HAVE_NETINET_IN_H
#define HAVE_ARPA_INET_H
#define HAVE_UNISTD_H
#define HAVE_SYS_TIME_H
#define HAVE_STDLIB_H

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/types.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include<netdb.h>	//hostent (Für Namensauflösung)

#include <string>

class ssh2_command
{
	private: //Data
//################################################################# Data ########################################################################
     
		std::string * hostname; 		// = "127.0.0.1";
		std::string * commandline; 		// = "uptime";
		std::string * username;			// = "user";
		std::string * password;    		// = "password";
    		unsigned long hostaddr;
    		int sock;
    		struct sockaddr_in sin;
    		const char *fingerprint;
    		LIBSSH2_SESSION *session;
    		LIBSSH2_CHANNEL *channel;
    		int rc;
    		int exitcode;
    		char *exitsignal;		//=(char *)"none";
    		int bytecount;			// = 0;
    		size_t len;
    		LIBSSH2_KNOWNHOSTS *nh;
    		int type;
		int startupDone;

		std::string output;
 
//###############################################################################################################################################

//############################################################## Con-/Destruktor ################################################################

	public:
    		ssh2_command(const char * host , const char * user , const char * pass , const char * command );
		ssh2_command(const ssh2_command &rcco_Obj);
		~ssh2_command();

//###############################################################################################################################################

//####################################################### Private Funktions #####################################################################

	private:
		int waitsocket(int socket_fd, LIBSSH2_SESSION *session);
		void startup();
		void sendCommand();
		void read();
		void cleanup();
        	int  hostname_to_ip(const char * hostname , char * ip);

//###############################################################################################################################################

//########################################################## Public Funktions ###################################################################

	public:
		char * execute();
		int giveOutputSize();
		const char * giveHostname();

//###############################################################################################################################################

};
