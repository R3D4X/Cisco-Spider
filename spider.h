#include "pcre_object.h"
#include "SSH_Command.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <deque>
#include <string>

//#define DEBUG 1

class spider
{
private:
	std::deque<ssh2_command *> 	pm_sdqsshcomp_got2crawl;				//Liste an SSH_Befehlen die auszuführen sind auf einem gewissen Host
												// Der Host wird bei der Initialisierung mitangegeben, also ist das die Liste
												// der zu crawlenden Hosts...

	pcre_object * 			pm_ppcreObj_hostHunter;					//pcre-objekt das nach Hostnamen in der Ausgabe des SSH-Befehls sucht

	std::vector<std::string *> 	pm_svssp_dontCrawl;					//Liste mit Hostnamen die nicht (nicht mehr) gecrawlt werden sollen

	std::string			pm_ss_result;						//String der das Ergebnis buffert
	
	std::string			pm_ss_totalResult;					//String der das gesamte Ergebnis buffert

	FILE * 				pm_fp_resultFile;					//Datei in die das Ergebnis geschrieben wird

	const char * 			pm_cp_username;						//Username der verwendet wird !!!Nur lesende Berechtigung geben!!!
												// oder halt wie immer das minimum an Rechten
	const char * 			pm_cp_password;						//Passwort das verwendet wird

public:
	spider( const char * host_root , const char * user , const char * pass , const char * regexFile , const char * resultFile);		//Konstruktor
	~spider();																//Destruktor

private:
	void dokumentResult();									//Dokumentiert das Ergebnis im Ergebnis-Buffer
	void addToGot2Crawl(const char * target);						//Füge einen neuen Host in die "TODO"-Liste hinzu
	void addToDontCrawl(const char * target);						//Füge einen Host in die Liste der Hosts die nicht gecrawlt werden sollen hinzu

public:
	void crawlNext();									//Erledige ein "TODO"
	unsigned int giveHostsLeft();								//Übergib die Anzahl an noch zu crawlenden Hosts
	void giveStatus();									//Gib den derzeitigen Stand aus
	const char * giveResults();								//Übergib den Ergebnis-Buffer
	void saveResults();									//Speicher den Ergebnis-Buffer in der Datei
};
