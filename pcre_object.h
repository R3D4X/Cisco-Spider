#ifndef PCRE_H
#define PCRE_H

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#define PCRE_VAL_SIZE   1024
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <string>
#include <vector>

class pcre_object
{

//############################### Data ##############################################
private:
    std::string 			pm_ss_regexPattern;

    pcre2_code*         		pm_pcreCode_compPattern;	//Compiltes pattern (mit ihm wird später gematcht)

    unsigned int 			pm_i_numberOfMatches;

    std::vector<std::string> 		pm_svss_matches;


//###################################################################################

//############################### Private ###########################################
private:
    void 			read_regexFile(const char* regs_file);

    void 			build_pcre(const char* regs_file);

//###################################################################################

//############################### Public ############################################
public:
    pcre_object(const char * name_der_regex_datei);

    ~pcre_object();

    void match_pcre(char * string);	//Die heilige Match-Fkt <3

    char * 	give_match(unsigned int number_of_match);

    unsigned int 	give_number_of_matches();

//###################################################################################

};
