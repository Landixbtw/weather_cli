#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>


void show_usage(char *PROGRAMM_NAME)
{
    fprintf(stdout,
	    "\nUsage: \e[1m%s [city]\e[0m\r\n\r\n"

	    "A command-line tool to display weather information about a city\r\n\r\n"

	    "For city names capitalization does not matter. \r\n"

	    "Washington or washington. \r\n"

	    "For cities that have a [space] in between you have to enter a plus.\r\n"

	    "New+York or new+york.\r\n"
	    "\n"
	    , PROGRAMM_NAME);
}

#endif
