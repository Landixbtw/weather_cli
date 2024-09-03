#include <curl/urlapi.h>
#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"

#include <curl/curl.h>
#include <curl/easy.h>

#define MAX_URL_LENGTH 256

/* 
 * To give the user the weather info he wants we need to get a city name.
 * */

char *BASE_URL = "http://api.weatherstack.com/current";

/* https://curl.se/libcurl/c/url2file.html */

void write_data(void) 
{
    
}


int main(void) 
{
    CURL *curl = curl_easy_init();
    CURLcode res;


    /* to have "access" to the key, we first need to load/read the file into memory */

    FILE *read_api_key_file;

    char *api_key_filename = "WEATHERSTACK_API_KEY.env";
    read_api_key_file = fopen(api_key_filename, "r");

    if (!read_api_key_file) {
        fprintf(stderr, "Error: Couldn't open file: %s\n", api_key_filename);
        return 1;
    }

    char ACCESS_KEY[124];


    int int_key = fscanf(read_api_key_file, "%s", ACCESS_KEY);


    fprintf(stdout, "api key: %s \n", ACCESS_KEY);

    fprintf(stdout, "For what city would you like to get info ? \n");
    fprintf(stdout, "If your city name is seperated by a [space] use a plus sign in between e.g. New+York. \n");

    /* NOTE: This gives us the user input */

    char CITY[124];

    scanf("%s", CITY);

    /*
     * we first need to make a call to the weatherstack api to get the json data
     *
     * https://curl.se/libcurl/c/libcurl-tutorial.html
    */


    // stackoverflow.com/questions/27422918/send-http-get-request-using-curl-in-c
    if (curl)
    {
        char url[MAX_URL_LENGTH];

        /* We are putting together the url, with the base url the secret api key, and the user input city */

        snprintf(url, sizeof(url), "%s?access_key=%s&query=%s", BASE_URL, ACCESS_KEY, CITY);

        fprintf(stdout, "url: %s\n", url);
        printf("\n");
        printf("\n");

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

        /* Perform the request, res gets the return code */
        res = curl_easy_perform(curl);

        /* Then we check for errors */

        if(res != CURLE_OK) {
            fprintf(stderr,"curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
        }

        /* CLEANUP ! */
        curl_easy_cleanup(curl);
    }

    /* 
     * To read json data we need to 
     * open the file
     * read the contents into a string
     * parse the JSON data
     * access the data
     * delete the json object 
     *
     * https://www.geeksforgeeks.org/cjson-json-file-write-read-modify-in-c/
     */

    return 0;
}
