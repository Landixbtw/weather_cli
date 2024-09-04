#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "cJSON.h"

#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/urlapi.h>

#define MAX_URL_LENGTH 256

/* 
 * To give the user the weather info he wants we need to get a city name.
*/

char *BASE_URL = "http://api.weatherstack.com/current";

/* https://curl.se/libcurl/c/url2file.html
 *
 *
 * The function itself is a wrapper around the standard C library function fwrite. Here's what it does:
 * It takes four parameters:
 * ptr: A pointer to the data to be written
 * size: The size of each element to be written
 * nmemb: The number of elements to write
 * stream: A pointer to the FILE stream where data will be written

 * It calls fwrite with these parameters, casting stream to a FILE* pointer.
 * It returns the number of elements successfully written.
 *
*/

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    /* (FILE *) stream  is a so called cast operation. In this case the cast basically says
     *
     * NOTE: I got help from claude, explaining this function. I obviously, 
     * took it from the libcurl website but I understand it thanks to claude explaining it.
     *
     * "I know this void * pointer is actually pointing to a FILE structure, so it's safe to treat it as a FILE * "
    */
    size_t written = fwrite(ptr, size, nmemb, (FILE *) stream);

    if (written != nmemb) {
        fprintf(stderr, "fwrite() failed: %s\n", strerror(errno));
    }
    return written;
}

int main(int argc, char *argv[]) 
{
    CURL *curl = curl_easy_init();
    CURLcode res;

    /* to have "access" to the key, we first need to load/read the file into memory */

    FILE *read_api_key_file;
    FILE *temp_json_file;

    const char *api_key_filename = "WEATHERSTACK_API_KEY.env";
    read_api_key_file = fopen(api_key_filename, "r");

    if (!read_api_key_file) {
        fprintf(stderr, "Error: Couldn't open file: %s\n", api_key_filename);
        return 1;
    }

    char ACCESS_KEY[124];
    int int_key = fscanf(read_api_key_file, "%s", ACCESS_KEY);

    // fprintf(stdout, "api key: %s \n", ACCESS_KEY);
    fprintf(stdout, "For what city would you like to get info ? \n");
    fprintf(stdout, "If your city name is seperated by a [space] use a plus sign in between e.g. New+York. \n");

    /* This gives us the user input */

    // TODO: check max input argc ?
    char CITY[124];
    scanf("%s", CITY);

    // FIX: Error handling for cities that dont exist / and or partial city names ? Api just gives a city with the same few letters

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

        // fprintf(stdout, "url: %s\n", url);
        printf("\n");

        curl_easy_setopt(curl, CURLOPT_URL, url);

    // TODO: For any error, rempromt the user, before writing anything to a file. Or printing anything to the screen

        /* For write_data to actually write data, we first need to open a file, and after write_data has written data we need to close the file again. */

        // wb = write binary
        temp_json_file = fopen("json_data.json", "wb");

        if (temp_json_file) {

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, temp_json_file);

            /* Perform the request, res gets the return code */
            res = curl_easy_perform(curl);

            /* Then we check for errors */
            if(res != CURLE_OK) {
                fprintf(stderr,"curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
                // here we need to rempromt the user 
            }

        }

        /* ❗ CLEANUP ❗ */
        curl_easy_cleanup(curl);
        fclose(temp_json_file);
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
     *
     * printing / parsing json. https://github.com/DaveGamble/cJSON?tab=readme-ov-file#printing-json
     */

    // temp_json_file = fopen("json_data.json", "rb"); 
    
    // if (temp_json_file) {
        // cJSON *json = cJSON_ParseWithLength(temp_json_file, MAX_URL_LENGTH);

        // cJSON_Delete(json);
        //
        // - read the contents into a string
    // }

    return 0;
}
