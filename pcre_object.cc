#include "pcre_object.h"

#define DEBUG 1

//############################### Private ###########################################

void pcre_object::read_regexFile(const char* regs_file)		//Ließt aus Config File die Regex-Pattern :D
{
    	int             size;
	int 		lineCnt = 0;
    	char            string[PCRE_VAL_SIZE+1];
    	FILE*           fp;
    	char*           pattern;

    	fp = fopen(regs_file, "r");
    	if( !fp ) 
	{
       		fprintf(stderr,"file open error : %s\n", regs_file);
        	exit(-1);
    	}

    	pm_ss_regexPattern[0] = '\0';

    	while( fgets(string, PCRE_VAL_SIZE, fp) != NULL ) 
	{
        	size = strlen(string);
        	if(string[size-1] == '\n')
		{
            		string[size-1] = '\0';
            		--size;
        	}
        	if(size > 1 && string[size-1] == '\r')
		{
            		string[size-1] = '\0';
            		--size;
        	}
        	if(string[0] == '\0')
		{
			//Do nothing
		}
		else
		{
			if(lineCnt == 0)
			{
				//Nothing :)
			}
			else
			{
		       		pm_ss_regexPattern += "|";
			}
			pm_ss_regexPattern += string;
			lineCnt++;
		}
    	}

    	fclose(fp);

#ifdef DEBUG
	std::cout << "Pattern: " << pm_ss_regexPattern.c_str() << std::endl;
#endif

    	//return pm_ss_regexPattern.c_str();		//Die ganze Schose für n const char * der returnt wird :)

}

void pcre_object::build_pcre(const char * regs_file)	//Compilen von den Regex-Patterns
{
    	PCRE2_SIZE          erroffset;
    	int                 errorcode;
    	PCRE2_UCHAR8        buffer[120];
	
    	read_regexFile(regs_file);		//Hier wird die Regex-File eingelesen
        
    	if( pm_ss_regexPattern.size() <= 0 ) 
        {
        	exit(-1);
    	}

    	pm_pcreCode_compPattern = pcre2_compile((PCRE2_SPTR) pm_ss_regexPattern.c_str() , -1, PCRE2_UTF, &errorcode, &erroffset, NULL);	//In pcre2_code* re wird das compilte pattern returnt
    	if ( pm_pcreCode_compPattern == NULL ) 
        {
        	(void)pcre2_get_error_message(errorcode, buffer, 120);
        	fprintf(stderr,"%d\t%s\n", errorcode, buffer);
		exit(-1);
    	}
}

//###################################################################################

//############################### Public ############################################

pcre_object::pcre_object(const char * name_der_regex_datei)
{
   	build_pcre(name_der_regex_datei); 
}

pcre_object::~pcre_object()	//Lel der gute alte destruktor
{
    	if( pm_pcreCode_compPattern ) 
    	{
    		pcre2_code_free(pm_pcreCode_compPattern);
    	}
    	if(pm_ss_regexPattern.size() > 0)
        {
            pm_ss_regexPattern.clear();
        }
        pm_svss_matches.clear();
}

void pcre_object::match_pcre(char* string)	//Die heilige Match-Fkt <3
{
    	pcre2_match_data*   match_data;
    	PCRE2_SIZE*         ovector;
	PCRE2_SIZE	    start_offset = 0;
    	int                 rc = 1;
    	int                 i;
    	int                 begin;
    	unsigned int        len;
    	char                chr;
        
        if(pm_svss_matches.size() > 0)
        {
            pm_svss_matches.clear();
        }
        pm_i_numberOfMatches = 0;

    	match_data = pcre2_match_data_create(3*10, NULL);			//"Allokiert" eine match_data matrix?


	while(rc > 0)								//Solange kein Fehler beim Matchen 
	{
    		rc = pcre2_match( 	pm_pcreCode_compPattern , 		//Vorher eingelesenes pattern
					(PCRE2_SPTR)string, 			//String der durchsucht wird
					strlen(string) , 			//Länge des Strings der durchsucht wird
					start_offset , 				//Startoffset, der jedesmal erhöht wird
					0, 					//Optionen
					match_data, 				//Matchdata (Arbeisspeicher von pcre, hier liegen dann auch die Ergebnisse
					NULL);					//Match-Context... Kein blassen Schimmer
										//
										//Schreibt die Matches in match_data, wandelt die Eingabe von char* in einen PCRE2_SPTR*

    		if( rc > 0 ) 								//Wenn ein Match
        	{
        		ovector = pcre2_get_ovector_pointer(match_data);		//offset vector
											//Aufteilung pcre match_data:
											//
											//	1      -      6    7  	 -      9
											//	captured data      workspace(pcre)
											//	2 x Integer
        		for(i = 0; i < rc; i++) 
            		{
            			begin = ovector[2*i];					//Anfang des Matches
            			len   = ovector[2*i+1] - ovector[2*i];			//Länge des Matches
            			chr = string[begin+len];				//Speicher das letze Zeichen des Matches (da nicht Null-Terminiert)
            			string[begin+len] = '\0';				//Terminiere den Match mit NULL
#ifdef DEBUG
            			fprintf(stdout, "%d\t%d\t%d\tString: %s\n", i, begin, begin+len, &string[begin]);
#endif
                    		pm_svss_matches.push_back(&string[begin]);		//Speicher den Match in unserem std::vector<std::string>
                    		pm_i_numberOfMatches++;					//Erhöhe den Match counter
            			string[begin+len] = chr;				//Spiele das ursprüngliche Zeichen wieder zurück
        		}
			start_offset = ovector[1];				//Der neue Offset ist der zweite Offset des Offset-Vectors... und wieder nach oben =D
    		}
	}
#ifdef DEBUG
    	fprintf(stdout, "\n");
	std::cout << "Found " << (rc-1) << " hosts :)" << std::endl;
#endif
    	pcre2_match_data_free(match_data);					//Dellokiere xD eine match_data matrix?
	if(string)
	{
		delete[] string;
		string = NULL;
	}
}

char * 	pcre_object::give_match(unsigned int number_of_match)
{
    if(pm_i_numberOfMatches > 0)
    {
    	if( (pm_i_numberOfMatches - 1) >= number_of_match )
    	{
		unsigned int size = pm_svss_matches[number_of_match].size();
        	char* returnString = new char[size + 1];
        
		std::copy( 	pm_svss_matches[number_of_match].begin(), 
				pm_svss_matches[number_of_match].end(), 
				returnString 
			 );
        
		returnString[size] = '\0';
		return returnString;
    	}
    	else
    	{
#ifdef DEBUG
    	        std::cerr << "Fuck you there's no element " << number_of_match << std::endl;
#endif
    	        return NULL;
    	}
    }
    else
    {
#ifdef DEBUG
	    std::cerr << "Fuck there aren't any elements!" << std::endl;
#endif
	    return NULL;
    }
}

unsigned int 	pcre_object::give_number_of_matches()
{
    return pm_i_numberOfMatches;
}

//###################################################################################
