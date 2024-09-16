
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

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
char *units = "m";
long http_code = 0;


// FIX: If the city the user enters and the city in the json dont match print error

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
void build_url(char *CITY);
size_t terminal_display_picture(const cJSON *current);

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
    
    if (int_key == 0) {
        fprintf(stderr, "No access key in file %s. \n", api_key_filename);
        return 1;
    }
    // fprintf(stdout, "api key: %s \n", ACCESS_KEY);

    /* This gives us the user input */
    if(argc == 2) {
        build_url(argv[1]);

        // fprintf(stdout, "The city you are checking is: %s\n", argv[1]);
    } else {
        fprintf(stderr, "Usage: %s <city>\nExample: %s New+York\n", PROGRAM_NAME, PROGRAM_NAME);
        fclose(read_api_key_file);
        return 1;
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
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

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
            } else {
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
                if (http_code >= 1 && http_code != 200) {
                    fprintf(stderr, "http error: %ld\n", http_code);
                }
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
     * parse the JSON data ✅
     * delete the json object ✅ 
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

    cJSON *json;

    /* 
     * claude helped me make the code a bit more efficient and easier to write, 
     *
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

    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL) {
        fprintf(stderr, "Error before: %s\n", error_ptr);
    }


    /* filter the usefull information and display it nicely */

    // NOTE: Error handling is being done above

    const cJSON *api_error = NULL;
    const cJSON *api_success = NULL;
    const cJSON *api_error_code = NULL;
    const cJSON *api_error_info = NULL;

    const cJSON *request = NULL;
    const cJSON *location = NULL;
    const cJSON *current = NULL;

    const cJSON *name = NULL;
    const cJSON *unit = NULL;
    const cJSON *observation_time = NULL;
    const cJSON *temperature = NULL;
    const cJSON *weather_descriptions = NULL;
    const cJSON *wind_speed = NULL;
    const cJSON *humidity = NULL;
    const cJSON *feelslike = NULL;

    /* We can't access name directly since its nested, so we first need to 
     * parse everything and then parse location 
    */


    /* This is for handling api error codes, and get a "custom" error message 
     * bzw. nicer looking, instead of the json 
    */

    // FIX: How to get the json error away ?

    api_success = cJSON_GetObjectItemCaseSensitive(json, "success");
    if (cJSON_IsFalse(api_success)) {
        api_error = cJSON_GetObjectItemCaseSensitive(json, "error");

        if (api_error != NULL) {
            api_error_code = cJSON_GetObjectItemCaseSensitive(api_error, "code");
            api_error_info = cJSON_GetObjectItemCaseSensitive(api_error, "info");
            if (cJSON_IsNumber(api_error_code) && (api_error_code->valueint) && cJSON_IsString(api_error_info) && (api_error_info->valuestring)) {
                fprintf(stderr, "API ERROR RESPONSE CODE: %i %s\n", api_error_code->valueint, api_error_info->valuestring);
            } else {
                fprintf(stderr, "Uknown error code.\n");
            }
        }
    }




    // TODO: How can this be more compact ? more better ?
    request = cJSON_GetObjectItemCaseSensitive(json, "request");
    if (request != NULL) {
        unit = cJSON_GetObjectItemCaseSensitive(request, "unit");

        // if (cJSON_IsString(unit) && (unit->valuestring != NULL)) {
        //     fprintf(stdout, "unit:\"%s\"\n", unit->valuestring);
        // }
    }

    location = cJSON_GetObjectItemCaseSensitive(json, "location");
    if (location != NULL) {
        name = cJSON_GetObjectItemCaseSensitive(location, "name");

        if (cJSON_IsString(name) && (name->valuestring != NULL )) {
            fprintf(stdout, "- City: %s\n", name->valuestring);
        }
    }

    current = cJSON_GetObjectItemCaseSensitive(json, "current");
    if (current != NULL) {
        observation_time = cJSON_GetObjectItemCaseSensitive(current, "observation_time");

        if (cJSON_IsString(observation_time) && (observation_time->valuestring != NULL)) {
            fprintf(stdout, "- Time: %s.\n", observation_time->valuestring);
        }

        temperature = cJSON_GetObjectItemCaseSensitive(current, "temperature");

        /* since temperature is a number we have to check for a number */
        if (cJSON_IsNumber(temperature)) {
            fprintf(stdout, "- Temperature: %i°C.\n", temperature->valueint);
        }

        weather_descriptions = cJSON_GetObjectItemCaseSensitive(current, "weather_descriptions");

        /* to print the part of the array, we first need to index, since there 
         * is only ever one entry in the array we can alway index to 0 
        */

        if (cJSON_IsArray(weather_descriptions)) {
            cJSON *weather_descriptions_array_item = cJSON_GetArrayItem(weather_descriptions, 0);
            if (cJSON_IsString(weather_descriptions_array_item) && (weather_descriptions_array_item->valuestring != NULL)) {
                fprintf(stdout, "- Weather: %s.\n", weather_descriptions_array_item->valuestring);
            }
        }

        wind_speed = cJSON_GetObjectItemCaseSensitive(current, "wind_speed");

        if (cJSON_IsNumber(wind_speed)) {
            fprintf(stdout, "- Wind speed: %i km/h.\n", wind_speed->valueint);
        }

        humidity = cJSON_GetObjectItemCaseSensitive(current, "humidity");
        
        if (cJSON_IsNumber(humidity)) {
            fprintf(stdout, "- Humidity: %i.\n", humidity->valueint);
        }

        feelslike = cJSON_GetObjectItemCaseSensitive(current, "feelslike");

        if (cJSON_IsNumber(feelslike)) {
            fprintf(stdout, "- Feels like: %i°C.\n", feelslike->valueint);
        }
    }


    // fprintf(stdout, "\n%s\n", json_string);


    // show the picture if possible


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
void build_url(char *CITY) 
{
    // NOTE: there is also the possibility to do query = fetch:ip, to pass the 
    // ip to the api, to get the weather for your location
    snprintf(url, sizeof(url), "%s?access_key=%s&query=%s", BASE_URL, ACCESS_KEY, CITY);
}

/* This function will check if you have the terminal image viewers installed, 
 * and if your terminal emulator can display a picture, if not you will be shown ascii.
*/
size_t terminal_display_picture(const cJSON *current) 
{

    // TODO: Determine if a cli of is installed, 
    // if there is no cli there or the terminal cant display the picture try ascii
    //
    /* Terminal Image viewers 
     *  timg
     *  chafa
    */

    size_t result = 0;
    const cJSON *picture= NULL;
    char command[124];
    const char *image_viewers[] = {"timg" , "chafa"};

    // NOTE: Alactritty seems to show the image but at a really bad quality
    const char *supported_terminals[] = { "xterm-ghostty", "kitty", "wezterm"};

    // TODO: Check with terminal emulator the user is using
    //
    // smth like this: https://askubuntu.com/questions/210182/how-to-check-which-terminal-emulator-is-being-currently-used
    if(current != NULL) {
        picture = cJSON_GetObjectItemCaseSensitive(current, "picture");
        if (cJSON_IsString(picture) && (picture->valuestring != NULL)) {
            snprintf(command, sizeof(command), "%s > /dev/null", picture->valuestring);
            result = system(command);
        }
    }

    if (result == -1) {
        fprintf(stderr, "Couldn't open picture with detected image viewer.\n");
        return 1;
    } else {
        system(command);
        return 0;
    }
    return 0;
}
