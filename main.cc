#include <iostream>
#include <string.h>

#include "spider.h"

int main(int argc , char ** argv)
{
	if(argc == 4)
	{
		int end = -1;
		spider spider_obj(argv[1] , argv[2] , argv[3] , "regex.file" , "Ergebnis.csv");
		while(end == -1)
		{
			spider_obj.crawlNext();
			spider_obj.giveStatus();
			spider_obj.saveResults();
			std::cout << "spider_obj.giveResults() = " << spider_obj.giveResults() << std::endl;
			if(spider_obj.giveHostsLeft() == 0)
			{
				end = 1;
			}
		}
	}
	else
	{
		std::cout << "Usage: <host> <username> <password>" << std::endl;
	}
}
