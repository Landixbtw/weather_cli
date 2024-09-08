#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/urlapi.h>

#include "cJSON.h"

#define MAX_URL_LENGTH 256
#define PROGRAM_NAME "./weather_cli"

const char *api_key_filename = "WEATHERSTACK_API_KEY.env";
const char *json_filename = "json_data.json";

/* 
 * To give the user the weather info he wants we need to get a city name.
*/

char ACCESS_KEY[124];
char *BASE_URL = "http://api.weatherstack.com/current";
char url[MAX_URL_LENGTH];


size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
void build_url(char *CITY);


int main(int argc, char *argv[]) 
{
    CURL *curl = curl_easy_init();
    CURLcode res;

    /* to have "access" to the key, we first need to load/read the file into memory */

    FILE *read_api_key_file;
    FILE *temp_json_file;

    read_api_key_file = fopen(api_key_filename, "r");

    if (read_api_key_file == NULL) {
        fprintf(stderr, "Error: Couldn't open file: %s\n", api_key_filename);
        return 1;
    }

    int int_key = fscanf(read_api_key_file, "%s", ACCESS_KEY);

    // fprintf(stdout, "api key: %s \n", ACCESS_KEY);

    /* This gives us the user input */
    if(argc == 2) {
        build_url(argv[1]);

        fprintf(stdout, "The city you are checking is: %s\n", argv[1]);
    } else {
        fprintf(stderr, "Usage: %s <city>\nExample: %s New+York\n", PROGRAM_NAME, PROGRAM_NAME);
    }

    // FIX: Error handling for cities that dont exist / and or partial city names ? Api just gives a city with the same few letters

    /*
     * we first need to make a call to the weatherstack api to get the json data
     *
     * https://curl.se/libcurl/c/libcurl-tutorial.html
    */


    // stackoverflow.com/questions/27422918/send-http-get-request-using-curl-in-c
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        /* For write_data to actually write data, we first need to open a file,
         * and after write_data has written data we need to close the file again. 
        */

        // wb = write binary
        temp_json_file = fopen("json_data.json", "wb");

        if (temp_json_file) {

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, temp_json_file);

            /* Perform the request, res gets the return code */
            res = curl_easy_perform(curl);

            /* Then we check for errors */
            if(res != CURLE_OK) {
                fprintf(stderr,"\ncurl_easy_perform() failed: %s\n",curl_easy_strerror(res));
            }

        }

        /* ❗ CLEANUP ❗ */
        curl_easy_cleanup(curl);
        fclose(temp_json_file);
    }

    /* 
     * To read json data we need to 
     * open the file ✅
     * read the contents into a buffer ✅
     * parse the JSON data
     * access the data
     * delete the json object 
     *
     * https://www.geeksforgeeks.org/cjson-json-file-write-read-modify-in-c/
     *
     * printing / parsing json. https://github.com/DaveGamble/cJSON?tab=readme-ov-file#printing-json
     */


    temp_json_file = fopen("json_data.json", "rb"); 

    if (temp_json_file == NULL) {
        fprintf(stderr, "Error: Couldn't open file: %s\n", json_filename);
        fclose(temp_json_file);
        return 1;

    }

 
    // Get file size
    // this code is from claude ai

    fseek(temp_json_file, 0, SEEK_END);
    long file_size = ftell(temp_json_file);
    fseek(temp_json_file, 0, SEEK_SET);
    // ---------------


    // get a buffer, that is as big as the file.
    char *buffer = malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Couldn't allocate enough memory.\n");
        fclose(temp_json_file);
        return 1;
    }

    // this was recommened by claude for better/more error handling

    // size_t bytes_read = fread(buffer, 1, file_size, temp_json_file);
    //
    // if (bytes_read < (size_t)file_size) {
    //     fprintf(stderr, "Not all bytes were read: bytes_read: %zu < file_size: %ld\n", bytes_read, file_size);
    //     free(buffer);
    //     return 1;
    // }
    //
    // buffer[bytes_read] = '\0';  // Null-terminate the buffer
    //
    // printf("Buffer contents: %s\n", buffer);

    // -------------------------------------------
    cJSON *json;

    /* 
     * claude helped me make the code a bit more efficient and easier to write, 
    */

    /*
     * using file_size and not size_of(file_size) here is important, so as to not the 
     * size of the long file_size but the actuall file_size
     *
     * " Using sizeof(file_size) might not give you what you expect. Remember, 
     * file_size is likely a long type, so sizeof(file_size) would give you the 
     * size of a long, not the actual file size."
     *
    */  
    while (fread(buffer, file_size , 1, temp_json_file)) {
        json = cJSON_Parse(buffer);
    }

    if (json == NULL) {
        fprintf(stderr, "cJSON json is NULL.\n");
        cJSON_Delete(json);
    }

    char *json_string = cJSON_Print(json);

    if (json_string == NULL) {
        fprintf(stderr, "Failed to print json_string: %s.\n", json_string);
        cJSON_Delete(json);
    }

    // this doesnt get printed and causes segfault, because json_string is NULL
    fprintf(stdout, "%s", json_string);


    fclose(temp_json_file);
    free(buffer);
    free(json_string);
    cJSON_Delete(json);
    //
    // - read the contents into a string


    return 0;
}


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

/* We are putting together the url, with the base url the secret api key, and the user input city */
void build_url(char *CITY) {
    snprintf(url, sizeof(url), "%s?access_key=%s&query=%s", BASE_URL, ACCESS_KEY, CITY);
}

