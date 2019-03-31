#include "SSH_Command.h"

#define DEBUG 1

//class ssh2_command
     
/*
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
*/


//############################################################## Con-/Destruktor ################################################################

ssh2_command::ssh2_command(const char * host , const char * user , const char * pass , const char * command)
			: hostname(new std::string(host)) , username(new std::string(user)) , password(new std::string(pass)) , commandline(new std::string(command))
{
	exitsignal = (char *) "none";
	startupDone = -1;
}

ssh2_command::ssh2_command(const ssh2_command &rcco_Obj)
            : hostname(rcco_Obj.hostname) , username(rcco_Obj.username) , password(rcco_Obj.password) , commandline(rcco_Obj.commandline)
{
   	exitsignal = (char *) "none";
	startupDone = -1;
}

ssh2_command::~ssh2_command()
{
	if(startupDone == 1)
	{
#ifdef DEBUG
		printf("Starting destruction...\n");
#endif
    		while( (rc = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN )
    		{
        		waitsocket(sock, session);
    		}
#ifdef DEBUG
 		printf("Closed the channel...\n");
#endif
    		if( rc == 0 )
    		{
    		    	exitcode = libssh2_channel_get_exit_status( channel );
	
	        	libssh2_channel_get_exit_signal(channel, &exitsignal,
	
	                                        NULL, NULL, NULL, NULL, NULL);
	    	}
#ifdef DEBUG
	 	printf("Got exitcode and exitsignal...\n");
#endif
	    	if (exitsignal)
		{
	        	fprintf(stderr, "\nGot signal: %s\n", exitsignal);
		}
	    	else
		{
	        	fprintf(stderr, "\nEXIT: %d bytecount: %d\n", exitcode, bytecount);
		}
 
	    	libssh2_channel_free(channel);
#ifdef DEBUG
	 	printf("Freed channel...\n");
#endif

	    	channel = NULL;
#ifdef DEBUG
	    	printf("Done Construktor! Goin to cleanup-fkt...\n");
#endif
		    
	    	this->cleanup();

		if(hostname)
		{
			delete hostname;
			hostname = NULL;
		}
		if(commandline)
		{
			delete commandline;
			commandline = NULL;
		}
		if(username)
		{
			delete username;
			username = NULL;
		}
		if(password)
		{
			delete password;
			password = NULL;
		}
	}
	else
	{
		printf("You never even started this thing up...\n");
	}
}

//###############################################################################################################################################

//####################################################### Private Funktions #####################################################################

int ssh2_command::waitsocket(int socket_fd, LIBSSH2_SESSION *session)
{
    struct timeval timeout;
    int rc;
    fd_set fd;
    fd_set *writefd = NULL;
    fd_set *readfd = NULL;
    int dir;
 
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
 
    FD_ZERO(&fd);
 
    FD_SET(socket_fd, &fd);
 
    /* now make sure we wait in the correct direction */ 
    dir = libssh2_session_block_directions(session);

 
    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;
 
    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;
 
    rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);
 
    return rc;
}

void ssh2_command::startup()
{
    rc = libssh2_init (0);

    if (rc != 0) {
        fprintf (stderr, "libssh2 initialization failed (%d)\n", rc);
	this->cleanup();
	exit(-1);
    }
    
    char * ip_addr = new char[16];
    
    hostname_to_ip(hostname->c_str() , ip_addr);
 
    hostaddr = inet_addr(ip_addr);

    printf("Connecting to %s\n" , ip_addr);

    delete [] ip_addr;
 
    /* Ultra basic "connect to port 22 on localhost"
     * Your code is responsible for creating the socket establishing the
     * connection
     */ 
    sock = socket(AF_INET, SOCK_STREAM, 0);
 
    sin.sin_family = AF_INET;
    sin.sin_port = htons(22);
    sin.sin_addr.s_addr = hostaddr;
    if (connect(sock, (struct sockaddr*)(&sin),
                sizeof(struct sockaddr_in)) != 0) 
    {
        fprintf(stderr, "failed to connect!\n");
	exit(-1);
    }
 
    /* Create a session instance */ 
    session = libssh2_session_init();

    if (!session)
    {
	fprintf(stderr, "No session created!\n");
	exit(-1);
    }
 
    /* tell libssh2 we want it all done non-blocking */ 
    libssh2_session_set_blocking(session, 0);

 
    /* ... start it up. This will trade welcome banners, exchange keys,
     * and setup crypto, compression, and MAC layers
     */ 
    while ( (rc = libssh2_session_handshake(session, sock)) == LIBSSH2_ERROR_EAGAIN ); 
    if (rc) 
    {
        fprintf(stderr, "Failure establishing SSH session: %d\n", rc);
	exit(-1);
    }
 
    nh = libssh2_knownhost_init(session);

    if(!nh) {
        /* eeek, do cleanup here */ 
        fprintf(stderr, "No known hosts found!\n");
	exit(-1);
    }
 
    /* read all hosts from here */ 
    libssh2_knownhost_readfile(nh, "known_hosts", LIBSSH2_KNOWNHOST_FILE_OPENSSH);
 
    /* store all known hosts to here */ 
    libssh2_knownhost_writefile(nh, "dumpfile", LIBSSH2_KNOWNHOST_FILE_OPENSSH);
 
    fingerprint = libssh2_session_hostkey(session, &len, &type);

    if(fingerprint) 
    {
        struct libssh2_knownhost *host;
#if LIBSSH2_VERSION_NUM >= 0x010206
        /* introduced in 1.2.6 */ 
        int check = libssh2_knownhost_checkp(nh, hostname->c_str(), 22,

                                             fingerprint, len,
                                             LIBSSH2_KNOWNHOST_TYPE_PLAIN|
                                             LIBSSH2_KNOWNHOST_KEYENC_RAW,
                                             &host);
#else
        /* 1.2.5 or older */ 
        int check = libssh2_knownhost_check(nh, hostname->c_str(),

                                            fingerprint, len,
                                            LIBSSH2_KNOWNHOST_TYPE_PLAIN|
                                            LIBSSH2_KNOWNHOST_KEYENC_RAW,
                                            &host);
#endif
        fprintf(stderr, "Host check: %d, key: %s\n", check,
                (check <= LIBSSH2_KNOWNHOST_CHECK_MISMATCH)?
                host->key:"<none>");
 
        /*****
         * At this point, we could verify that 'check' tells us the key is
         * fine or bail out.
         *****/ 
    }
    else 
    {
        /* eeek, do cleanup here */ 
        fprintf(stderr, "No fingerprint found!\n");
	exit(-1);
    }
    libssh2_knownhost_free(nh);

 
    if ( password->size() != 0 ) 
    {
        /* We could authenticate via password */ 
        while ( (rc = libssh2_userauth_password(session, username->c_str(), password->c_str())) == LIBSSH2_ERROR_EAGAIN );
        if (rc) 
	{
            	fprintf(stderr, "Authentication by password failed.\n");
		this->cleanup();
	  	exit(-1);
        }
    }
    else 
    {
        /* Or by public key */ 
        while ((rc = libssh2_userauth_publickey_fromfile(session, username->c_str(),

                                                         "/home/user/"
                                                         ".ssh/id_rsa.pub",
                                                         "/home/user/"
                                                         ".ssh/id_rsa",
                                                         password->c_str())) ==
               LIBSSH2_ERROR_EAGAIN);
        if (rc) 
	{
            	fprintf(stderr, "\tAuthentication by public key failed\n");
		this->cleanup();
		exit(-1);
        }
    }
 
#if 0
    libssh2_trace(session, ~0 );

#endif
 
    /* Exec non-blocking on the remove host */ 
    while( (channel = libssh2_channel_open_session(session)) == NULL &&

           libssh2_session_last_error(session,NULL,NULL,0) ==

           LIBSSH2_ERROR_EAGAIN )
    {
        waitsocket(sock, session);
    }
    if( channel == NULL )
    {
        fprintf(stderr,"Error\n");
        exit( 1 );
    }
		
    startupDone = 1;

//###############################################################################################################################################
}
		
void ssh2_command::sendCommand()
{
     while( (rc = libssh2_channel_exec(channel, commandline->c_str())) ==

           LIBSSH2_ERROR_EAGAIN )
    {
        waitsocket(sock, session);
    }
    if( rc != 0 )
    {
        fprintf(stderr,"Error\n");
        exit( 1 );
    }   
}
		
void ssh2_command::read()
{
	int end = -1;
    	while(end == -1)
    	{
        	/* loop until we block */ 
        	int rc = 101; 
        	while( rc > 0 )
        	{
            		char buffer[0x4000];						//Buffer in den die Ausgabe des Channels geschrieben wird
            		rc = libssh2_channel_read( channel, buffer, sizeof(buffer) );

            		if( rc > 0 )
            		{
                		int i;
                		bytecount += rc;
#ifdef DEBUG
                		fprintf(stderr, "We read:\n");
#endif
                		for( i=0; i < rc; ++i )
				{
#ifdef DEBUG
                    			fputc( buffer[i], stderr);
#endif
					output.push_back(buffer[i]);
				}
#ifdef DEBUG
                		fprintf(stderr, "\n");
#endif
            		}
            		else 
			{
                		if( rc != LIBSSH2_ERROR_EAGAIN )
                    		/* no need to output this for the EAGAIN case */ 
                   	 	{
#ifdef DEBUG
					fprintf(stderr, "libssh2_channel_read returned %d\n", rc);
#endif
				}
            		}
        	}
        	/* this is due to blocking that would occur otherwise so we loop on
           	   this condition */ 

		if( rc == LIBSSH2_ERROR_EAGAIN )
        	{
            		waitsocket(sock, session);
        	}
        	else
		{
            		end = 1;
		}
    	}
    	exitcode = 127;
}

void ssh2_command::cleanup()
{
    libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
    libssh2_session_free(session);

#ifdef WIN32
    closesocket(sock);
#else
    close(sock);
#endif
    fprintf(stderr, "all done\n");
 
    libssh2_exit();
}

int ssh2_command::hostname_to_ip(const char * hostname , char* ip)
{
	struct hostent *he;
	struct in_addr **addr_list;
	int i;
		
	if ( (he = gethostbyname( hostname ) ) == NULL) 
	{
		// get the host info
		herror("gethostbyname");
		return 1;
	}

	addr_list = (struct in_addr **) he->h_addr_list;
	
	for(i = 0; addr_list[i] != NULL; i++) 
	{
		//Return the first one;
		strcpy(ip , inet_ntoa(*addr_list[i]) );
		return 0;
	}
	return 1;
}

//###############################################################################################################################################

//########################################################## Public Funktions ###################################################################

char * ssh2_command::execute()
{
	this->startup();
#ifdef DEBUG
	printf("Started up!\n");
#endif
	this->sendCommand();
#ifdef DEBUG
	printf("Send command...\n");
#endif
	this->read();
#ifdef DEBUG
	printf("Read output!\n");
#endif

	unsigned int size = output.size();
        char* returnString = new char[size + 1];
        
	std::copy( 	output.begin(), 
			output.end(), 
			returnString 
		 );
        
	returnString[size] = '\0';
	return returnString;

}

int ssh2_command::giveOutputSize()
{
        return output.size();
}

const char * ssh2_command::giveHostname()
{
        return hostname->c_str();
}
//###############################################################################################################################################

