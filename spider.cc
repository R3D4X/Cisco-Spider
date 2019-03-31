#include "spider.h"

#define DEBUG 1

/*
 *
 *	private:
 *		std::vector<ssh_command *> 	pm_sdqsshcomp_got2crawl;
 *		pcre_object * 			pm_ppcreObj_hostHunter;
 *		std::vector<std::string  **> 	pm_svssp_dontCrawl;
 *		std::string			pm_ss_result;
 *		std::string			pm_ss_totalResult;
 *		FILE * 				pm_fp_resultFile
 *
 *		const char const * 		pm_cp_username;
 *		const char const * 		pm_cp_password;
 */

//############################### Private ###########################################

spider::spider( const char * host_root , const char * user , const char * pass , const char * regexFile , const char * resultFile)
		: pm_cp_username(user) , pm_cp_password(pass)
{
	pm_ppcreObj_hostHunter = new pcre_object( regexFile );

	pm_fp_resultFile = fopen(resultFile , "w+");
	if(pm_fp_resultFile == NULL)
	{
		perror("Error opening file: ");
		exit(-1);
	}

	addToGot2Crawl( host_root );
		
	pm_svssp_dontCrawl.push_back( new std::string(host_root) );							//Wird am Anfang leider 1 Mal gebraucht, da sonst die Abbruchbedingung in der Prüfung der
															// Liste der schon gecrawlten Hosts rummeckert... :D
}

spider::~spider()
{
	if(pm_fp_resultFile)
	{
		fputs(pm_ss_result.c_str() , pm_fp_resultFile);
		if(ferror(pm_fp_resultFile))
		{
			std::cerr << "An error occured during file write!" << std::endl;
			perror("Error: ");
		}
		fclose(pm_fp_resultFile);
	}

	if(pm_sdqsshcomp_got2crawl.size() > 0)
	{
		pm_sdqsshcomp_got2crawl.clear();
	}
	if(pm_ppcreObj_hostHunter)
	{
		delete pm_ppcreObj_hostHunter;
		pm_ppcreObj_hostHunter = NULL;
	}
	if(pm_svssp_dontCrawl.size() > 0)
	{
		pm_svssp_dontCrawl.clear();
	}

	if(pm_ss_result.size() > 0)
	{
		pm_ss_result.clear();
	}

	/*
	if(pm_cp_username)
	{
		delete [] pm_cp_username;
		pm_cp_username = NULL;
	}
	if(pm_cp_password)
	{
		delete [] pm_cp_password;
		pm_cp_password = NULL;
	}
	*/
}

//private:
void spider::dokumentResult()
{
        pm_ss_result += pm_sdqsshcomp_got2crawl[0]->giveHostname();
#ifdef DEBUG
	pm_ss_result += " is connected to ";
#endif
#ifndef DEBUG
	pm_ss_result += ";";
#endif
       	for( unsigned int i_cnt = 0 ; pm_ppcreObj_hostHunter->give_match(i_cnt) != NULL ; i_cnt++)
	{
		pm_ss_result += pm_ppcreObj_hostHunter->give_match(i_cnt);
		pm_ss_result += ";";
	}
#ifdef DEBUG
	pm_ss_result += " | ";
	std::cout << "Current result: " << pm_ss_result.c_str() << std::endl;
#endif
#ifndef DEBUG
	pm_ss_result += "\n";
#endif
}

void spider::addToGot2Crawl(const char * target)
{
/*
 *
 *	private:
 *		std::vector<ssh_command *> 	pm_sdqsshcomp_got2crawl;
 *		pcre_object * 			pm_ppcreObj_hostHunter;
 *		std::vector<std::string  *> 	pm_svssp_dontCrawl;
 *		std::string			pm_ss_result;
 *		fstream 			pm_fp_resultFile
 *
 *		const char const * 		pm_cp_username;
 *		const char const * 		pm_cp_password;
 *
 *		ssh2_command(const char * host , const char * user , const char * pass , const char * command );
 */


	pm_sdqsshcomp_got2crawl.push_back( new ssh2_command(target , pm_cp_username , pm_cp_password , "sh cdp neighbor") );
#ifdef DEBUG
	std::cout << "Added " << pm_sdqsshcomp_got2crawl.back()->giveHostname() << " to Got2Crawl!" << std::endl;
#endif
}

void spider::addToDontCrawl(const char * target)
{
	std::string candidate( target );										//Erstelle einen string aus dem Ziel
	int dontCrawl = -1;												//Setze die "Host schon in der Liste"-Variable auf -1 (falsch)
	for(unsigned int hostNr = 0 ; dontCrawl == -1 && hostNr < pm_svssp_dontCrawl.size() ; hostNr++)			//Führe das folgende für alle gecrawlten Hosts aus, brich ab wenn einer gleich dem neuen Kandidaten ist
	{
#ifdef DEBUG
		std::cout << pm_svssp_dontCrawl[hostNr]->c_str() << " =? " << candidate.c_str() << std::endl;
#endif
		if( pm_svssp_dontCrawl[hostNr]->compare( candidate ) == 0 )							//Vergleiche den gefundenen Host mit den gecrawlten Hosts
		{
			dontCrawl = 1;													//Wenn ein gecralter Host = gefundener Host -> "Host schon gecrawlt"-Variable ist 1 (wahr)
		}
#ifdef DEBUG
		std::cout << "Result: " << dontCrawl << std::endl;
#endif
	}
	if( dontCrawl == -1 )												//Wenn "Host schon in der Liste"-Variable auf -1 (falsch)
	{
		pm_svssp_dontCrawl.push_back( new std::string(target) );							//Füge den Kandidaten in die Liste der schon gecrawlten Hosts hinzu
	}
}

//###################################################################################

//############################### Public ############################################

void spider::crawlNext()
{
//++++++++++++++++++++++++++++++++++++++ Children and Parent Qualitytime <3 +++++++++++++++++++++++++++++++++++++++++++
//
//
//
//
// temporär:
		int childCnt = 5;
		int currentChilds = 0;
//
//
	pid_t pid;

	int commPipe[10];	//ChildCnt * 2
	int ackPipe[10];	//ChildCnt * 2

	bool gotPipe = false;
	bool gotChildren = false;

	int   exitStatus = 0;

	int pipeRet = 0;
	
	for(int i_cnt = 0 ; i_cnt < childCnt ; i_cnt++)
	{
		pipeRet +=pipe(&commPipe[2*i_cnt]);
		pipeRet += pipe(&ackPipe[2*i_cnt]);
	}

	std::deque<std::string *> sdqss_foundHostnames;
       
	if(pipeRet < 0)
	{
		perror("Jeeeez broke meeese broke da pipe :( -> ");
		gotPipe = false;
	}
	else
	{
		gotPipe = true;
	}

	/*
	 *
	 * 	Hier schleife zum Children hochzählen
	 *
	 */


	if(gotPipe == true)						//Bevor geforkt wird wird der zu Crawlende Host der dontCrawl-Liste hinzugefügt
	{
		do
		//while( (currentChilds < childCnt) && pid > 0 && pm_sdqsshcomp_got2crawl.size() > 0 );
		//for( ; currentChilds < 5 ; currentChilds++);
		{

			addToDontCrawl( pm_sdqsshcomp_got2crawl.front()->giveHostname() );						//Füge den Hostnamen auf den wir uns connecten zu der Liste der gecrawlten Hosts hinzu

			pid = fork();								//<--------------- POF (Point of Fork)

			if(pid == -1)
			{
				perror("Played with a fork... Yet no children :/ ->");
				exit(EXIT_FAILURE);
			}
			else if( pid == 0)
			{
#ifdef DEBUG
				std::cout << "Hi im child No. " << currentChilds << std::endl;
#endif
			}
			else
			{
#ifdef DEBUG
				std::cout << "Hi im the Parent and made child No. " << currentChilds << std::endl;
				std::cout << currentChilds  << " < " << childCnt << " && " << pid << " > 0 " << " && " << pm_sdqsshcomp_got2crawl.size() << " > 0" << std::endl;
#endif
				//currentChilds++;
				pm_sdqsshcomp_got2crawl.pop_front();										//Entferne den eben gecrawlten Host aus der Liste der zu crawlenden Hosts
				currentChilds++;
#ifdef DEBUG
				if(pm_sdqsshcomp_got2crawl.size() > 0)
				{
					std::cout << "Got2Crawl:" << std::endl;
					for(auto iter : pm_sdqsshcomp_got2crawl)
					{
						std::cout << iter->giveHostname() << std::endl;
					}
					std::cout << "-> Next: " << pm_sdqsshcomp_got2crawl.front()->giveHostname() << std::endl;
				}
#endif
			}
#ifdef DEBUG
				std::cout << "Status: " << currentChilds  << " < " << childCnt << " && " << pid << " > 0 " << " && " << pm_sdqsshcomp_got2crawl.size() << " > 0" << std::endl;
#endif

		}
		while( (currentChilds < childCnt) && pid > 0 && pm_sdqsshcomp_got2crawl.size() > 0 );
	}

	do
	{
		if(pid == 0)	//Children
		{
#ifdef DEBUG
			std::cout << "Child " << currentChilds << " goin to ssh : " << pm_sdqsshcomp_got2crawl.front()->giveHostname() << std::endl;
#endif
			close( commPipe[currentChilds * 2] );//Close the Parents part of the pipe
			close( ackPipe[(currentChilds * 2)+1] );//Close the Parents part of the pipe
			char * buffer = pm_sdqsshcomp_got2crawl.front()->execute();

			FILE * logFile;
			logFile = fopen("logFile.log" , "a");
			if(logFile == NULL)
			{
				perror("Error opening file: ");
			}
			fputs(buffer , logFile);
			fclose(logFile);

			pm_ppcreObj_hostHunter->match_pcre( pm_sdqsshcomp_got2crawl.front()->execute() );				//führe den Befehl aus und gib die Ausgabe an das pcre-objekt

			for( unsigned int i_cnt = 0 ; pm_ppcreObj_hostHunter->give_match(i_cnt) != NULL ; i_cnt++)			//Führe das folgende für alle Ergebnisse (gefundenen Hosts) aus...
			{
				std::string foundHost( pm_ppcreObj_hostHunter->give_match(i_cnt) );						//Erstelle einen string aus dem match
#ifdef DEBUG
				std::cout << "Child is writing to commPipe[" << ((currentChilds*2) + 1) << "]..." << std::endl;
#endif

				write( commPipe[(currentChilds * 2)+1] , foundHost.c_str() , foundHost.size() + 1 );
#ifdef DEBUG
				std::cout << "Child " << currentChilds << " wrote: " << foundHost.c_str() << std::endl;
#endif
			}
			write( commPipe[(currentChilds * 2)+1] , "FIN\0" , 4 );
#ifdef DEBUG
			std::cout << "Waiting for acknowlegement on ackPipe[" << ((currentChilds*2)+1) << "]..." << std::endl;
#endif
			char cp_buffer[100];
			bool end = false;
			while( end == false)
			{
				int readRet = read( ackPipe[currentChilds*2] , cp_buffer , sizeof(cp_buffer) ) > 0;	//Child searches for acknowledgement of its work...
				write( commPipe[(currentChilds * 2)+1] , "FIN\0" , 4 );
				//std::cout << "Child read: " << cp_buffer << std::endl;
				if(strcmp(cp_buffer , "ACK") == 0)
				{
					end = true;
				}
				if(readRet < 0)
				{
					end = true;
				}
			}
#ifdef DEBUG
			std::cout << "Child received: " << cp_buffer << " Closing the pipe..." << std::endl;
#endif
			close(commPipe[ (currentChilds * 2) + 1]);//Close childs pipe-part
			close(ackPipe[currentChilds * 2]);//Close childs pipe-part
			exit(EXIT_SUCCESS);
		}
		else		//Parent
		{
#ifdef DEBUG
			std::cout << "Im the parent :) and im goin to read from a pipe!" << std::endl;
#endif
			for( int i_cnt = 0 ; i_cnt < currentChilds ; i_cnt++)
			{
				close( commPipe[(i_cnt * 2)+1] );//Close child pipe-part
				close( ackPipe[i_cnt * 2] );//Close child pipe-part
				char cp_buffer[100];
				std::string * ss_buffer = new std::string;

				int   exitStatus = 0;

#ifdef DEBUG
				std::cout << "Parent is reading from commPipe[" << (i_cnt*2) << "]..." << std::endl;
#endif



				sleep(1);												//<-------------------------------------------
				


				bool end = false;
				while( end == false )
				{
#ifdef DEBUG
					std::cout << "Starting blocking read on pipe[" << (i_cnt*2) << "]..." << std::endl;
#endif
					read(commPipe[i_cnt*2] , cp_buffer , sizeof(cp_buffer));	//Then the parent reads all the pipe char by char
					std::cout << "Parent read:" << cp_buffer << std::endl;
					if( strcmp(cp_buffer , "FIN") == 0)
					{
						end = true;
					}
					else
					{
						*ss_buffer += cp_buffer;
					}
				}

#ifdef DEBUG
					std::cout << "Parent received:" << std::endl;
					std::cout <<"------------------------------------------------------------------------------- " << std::endl;
					std::cout << ss_buffer->c_str() << std::endl;
					std::cout << "------------------------------------------------------------------------------- " << std::endl;
#endif

				write( ackPipe[(i_cnt*2)+1] , "ACK\0" , 4 );

				sdqss_foundHostnames.push_back(ss_buffer);
#ifdef DEBUG
				std::cout << "Wrote to ackPipe[" << ((i_cnt*2)+1) << "]: ACK" << std::endl;
				perror("Read status: ");		
				perror("Exited: ");
				std::cout << "Thus the parent had enough ;) Goin to store our results. Shall we?" << std::endl;
#endif
				close( commPipe[i_cnt*2] );//Close parents pipe-part
				close( ackPipe[(i_cnt*2)+1] );//Close parents pipe-part
			}
		}
#ifdef DEBUG
		std::cout << "Executed command" << std::endl;
		std::cout << "Found: " << sdqss_foundHostnames.size() << " Hosts " << std::endl;
#endif
		for( auto iter : sdqss_foundHostnames)			//Führe das folgende für alle Ergebnisse (gefundenen Hosts) aus...
		{
#ifdef DEBUG
			std::cout << "Found host: " << iter->c_str() << std::endl;
#endif
			int dontCrawl = -1;											//Setze die "Host schon gecrawlt"-Variable auf -1 (falsch)
			for(unsigned int hostNr = 0 ; dontCrawl == -1 && hostNr < pm_svssp_dontCrawl.size() ; hostNr++)		//Führe das folgende für alle gecrawlten Hosts aus, brich ab wenn einer gleich dem gefundenen Host ist
			{
				if( pm_svssp_dontCrawl[hostNr]->compare( *iter ) == 0 )							//Vergleiche den gefundenen Host mit den gecrawlten Hosts
				{
					dontCrawl = 1;												//Wenn ein gecralter Host = gefundener Host -> "Host schon gecrawlt"-Variable ist 1 (wahr)
#ifdef DEBUG
					std::cout << iter->c_str() << " = " << pm_svssp_dontCrawl[hostNr]->c_str() << std::endl;
#endif
				}
			}
			if( dontCrawl == -1 )												//Wenn "Host schon gecrawlt"-Variable auf -1 (falsch)
			{
#ifdef DEBUG		
				std::cout << "Adding " << iter->c_str() << " to Got2Crawl" << std::endl;
#endif
				addToGot2Crawl( iter->c_str() );										//Füge den gefundenen Host in die Liste der zu crawlenden Hosts hinzua
			}
		}
		dokumentResult();												//Hänge das Resultat an den Ergebnis String
#ifdef DEBUG
		std::cout << "Got2Crawl:" << std::endl;
		for(auto iter : pm_sdqsshcomp_got2crawl)
		{
			std::cout << iter->giveHostname() << std::endl;
		}
		currentChilds--;
		std::cout << "Question is now: currentChilds: " << currentChilds << " > 0 -> Is there a child left?" << std::endl;
#endif
	}
	while( currentChilds > 0 );
#ifdef DEBUG
	perror("Exited: ");
#endif
}

unsigned int spider::giveHostsLeft()
{
	 return pm_sdqsshcomp_got2crawl.size();
}

void spider::giveStatus()
{
	if(pm_sdqsshcomp_got2crawl.size() > 0)
	{
		std::cout << "Crawling " << pm_sdqsshcomp_got2crawl[0]->giveHostname() << " still " << pm_sdqsshcomp_got2crawl.size() << " left." << std::endl;
	}
	else
	{
		std::cout << "Done =)" << std::endl;
	}
}
	
const char * spider::giveResults()
{
	pm_ss_totalResult.clear();

	rewind(pm_fp_resultFile);

	char c_in = '\r';
	while(c_in != EOF)
	{
		c_in = fgetc(pm_fp_resultFile);
		pm_ss_totalResult.push_back(c_in);
	}

	pm_ss_totalResult += pm_ss_result;
	pm_ss_totalResult.back() = '\0';
	return pm_ss_totalResult.c_str();
}
	
void spider::saveResults()
{

	fputs(pm_ss_result.c_str() , pm_fp_resultFile);
	pm_ss_result.clear();
#ifdef DEBUG
	std::cout << "Wrote " << pm_ss_result.c_str() << " to file" << std::endl;
#endif
}
