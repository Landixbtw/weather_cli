
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <curl/curl.h>
#include <curl/easy.h>

#include "../include/cJSON.h"
#include "../include/terminal_support.h"
#include "../include/image_to_ascii.h"
#include "../include/terminal_display_picture.h"

#define MAX_URL_LENGTH 256
#define PROGRAM_NAME "./weather_cli"


const char *api_key_filename = "src/resources/WEATHERSTACK_API_KEY.env";
const char *json_filename = "src/resources/json_data.json";

/* 
 * To give the user the weather info he wants we need to get a city name.
*/

char ACCESS_KEY[124];
char *BASE_URL = "http://api.weatherstack.com/current";
char url[MAX_URL_LENGTH];
char *units = "m";
long http_code = 0;


size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
void build_url(char *CITY);
//
void replace_umlaute(char *dest, wchar_t umlaut, size_t *j);
char *filter_char(const wchar_t input, size_t output_size);
//
char* transliterate_umlaut(const char* input);

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

    if (fscanf(read_api_key_file, "%s", ACCESS_KEY) != 1) {
        perror("ERROR: Failed to read API key from file\n");
        fclose(read_api_key_file);
        return 1;
    }

    if (!setlocale(LC_CTYPE, "de_DE.UTF-8")) {
        perror("Can't set the specified locale!\n Check LANG, LC_CTYPE, LC_ALL.\n");
        return 1;
    }

    char *url_string = NULL;
    /* This gives us the user input */
    if(argc == 2) { 
        char *transliterated = NULL;

        url_string = argv[1];
        url_string= transliterate_umlaut(argv[1]);

        build_url(url_string);
        free(transliterated);
    } else {
        fprintf(stderr, "Usage: %s <city>\nExample: %s New+York\n", PROGRAM_NAME, PROGRAM_NAME);
        fclose(read_api_key_file);
        return 1;
    }

    // FIX: Error handling for cities that dont exist / and or partial city names ? Api just autocompletes to some random city

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
        temp_json_file = fopen(json_filename, "wb+");

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


    temp_json_file = fopen(json_filename, "rb"); 

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

    // const cJSON *request = NULL;
    const cJSON *location = NULL;
    const cJSON *current = NULL;

    const cJSON *name = NULL;
    // const cJSON *unit = NULL;
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


    // FIX: How can this be more compact ? more better ?

    // request = cJSON_GetObjectItemCaseSensitive(json, "request");

    // if (request != NULL) {
        // unit = cJSON_GetObjectItemCaseSensitive(request, "unit");

        // if (cJSON_IsString(unit) && (unit->valuestring != NULL)) {
        //     fprintf(stdout, "unit:\"%s\"\n", unit->valuestring);
        // }
    // }

    fprintf(stdout, "Weather report for ");

    if (get_terminal_emulator_name() != 0) {
        image_to_ascii();
    } else {
        terminal_display_picture(current);
    }

    location = cJSON_GetObjectItemCaseSensitive(json, "location");
    if (location != NULL) {
        name = cJSON_GetObjectItemCaseSensitive(location, "name");

        if (cJSON_IsString(name) && (name->valuestring != NULL )) {
            fprintf(stdout, "\n\033[4m%s\033[0m\n", name->valuestring);
        }
    }

    current = cJSON_GetObjectItemCaseSensitive(json, "current");
    if (current != NULL) {
        observation_time = cJSON_GetObjectItemCaseSensitive(current, "observation_time");

        if (cJSON_IsString(observation_time) && (observation_time->valuestring != NULL)) {
            fprintf(stdout, "\033[4mObservation Time:\033[0m %s.\n", observation_time->valuestring);
        }

        temperature = cJSON_GetObjectItemCaseSensitive(current, "temperature");

        /* since temperature is a number we have to check for a number */
        if (cJSON_IsNumber(temperature)) {
            fprintf(stdout, "\033[4mTemperature\033[0m: %i°C.\n", temperature->valueint);
        }


        wind_speed = cJSON_GetObjectItemCaseSensitive(current, "wind_speed");

        weather_descriptions = cJSON_GetObjectItemCaseSensitive(current, "weather_descriptions");

        /* to print the part of the array, we first need to index, since there 
         * is only ever one entry in the array we can alway index to 0 
        */

        if (cJSON_IsArray(weather_descriptions)) {
            cJSON *weather_descriptions_array_item = cJSON_GetArrayItem(weather_descriptions, 0);
            if (cJSON_IsString(weather_descriptions_array_item) && (weather_descriptions_array_item->valuestring != NULL)) {
                fprintf(stdout, "\033[4mWeather\033[0m: %s. \n", weather_descriptions_array_item->valuestring);
            }
        }

        if (cJSON_IsNumber(wind_speed)) {
            fprintf(stdout, "\033[4mWind speed\033[0m: %i km/h.\n", wind_speed->valueint);
        }


        humidity = cJSON_GetObjectItemCaseSensitive(current, "humidity");

        if (cJSON_IsNumber(humidity)) {
            fprintf(stdout, "\033[4mHumidity:\033[0m %i%%.\n", humidity->valueint);
        }

        feelslike = cJSON_GetObjectItemCaseSensitive(current, "feelslike");

        if (cJSON_IsNumber(feelslike)) {
            fprintf(stdout, "\033[4mFeels like\033[0m: %i°C.\n", feelslike->valueint);
        }
    }

    image_to_ascii();
    // show the picture if possible

    fclose(read_api_key_file);
    fclose(temp_json_file);
    free(buffer);
    free(json_string);
    cJSON_Delete(json);

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
     * "I know this void * pointer is actually pointing to a FILE structure, so it's safe to treat it as a FILE * "
     *
     * NOTE: I got help from claude, explaining this function. I obviously, 
     * took it from the libcurl website but I understand it thanks to claude explaining it.
    */
    size_t written = fwrite(ptr, size, nmemb, (FILE *) stream);

    if (written != nmemb) {
        // fprintf(stderr, "fwrite() failed: %s\n", strerror(errno));
        perror("fwrite() failed");
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

// --------------------------------------------------------------------
// --------------------------------------------------------------------

enum umlaute {
    UMLAUT_BIG_A = 196,
    UMLAUT_BIG_O = 214,
    UMLAUT_BIG_U = 220,
    UMLAUT_SMALL_A = 228,
    UMLAUT_SMALL_O = 246, 
    UMLAUT_SMALL_U = 252,
};

/*  This function filters the characters it is given and determines if the wchar_t given
 *  is a umlaut or not, if the input character is a umlaut it replaces it with the "normal" letters 
*/
char *filter_char(const wchar_t input, size_t output_size) {
    // output size sizeof(wchar_t)

    char *output = "zero";

    size_t j = 0;
    // if input[i] UMLAUT A O U then replace else nothing

    switch(input) {
        case UMLAUT_BIG_A : replace_umlaute(output, input, &j);
        case UMLAUT_BIG_O : replace_umlaute(output, input, &j);
        case UMLAUT_BIG_U : replace_umlaute(output, input, &j);
        case UMLAUT_SMALL_A : replace_umlaute(output, input, &j);
        case UMLAUT_SMALL_O : replace_umlaute(output, input, &j);
        case UMLAUT_SMALL_U : replace_umlaute(output, input, &j);
        default : break ;
    }
    return output;
}


/* This function takes in the destination string and the char for ü ä ö, the size_t *j is there 
 * to keep track  of the current possition in the whole destination string
*/
void replace_umlaute(char *dest, wchar_t umlaut, size_t *j) {

    // NOTE: wchar_t umlaut has to be a char and cant be a string because the 
    // switch case statement doenst take a string
    const char *replacement;

    switch(umlaut) {
        case UMLAUT_BIG_A: replacement = "AE"; break;
        case UMLAUT_BIG_O: replacement = "OE"; break;
        case UMLAUT_BIG_U: replacement = "UE"; break;

        case UMLAUT_SMALL_A: replacement = "ae"; break;
        case UMLAUT_SMALL_O: replacement = "oe"; break;
        case UMLAUT_SMALL_U: replacement = "ue"; break;


        default : break; 
    }

    printf("-------------------\n");
    printf("UMLAUT_BIG_A: %i", UMLAUT_BIG_A);


    /* Die Klammern sind wichtig denn ohne die Klammern würden wir den 
     * Pointer erhöhen und nicht das value jetzt dereferencen wir und 
     * erhöhen dann das value
    */

    dest[(*j)++] = replacement[0];
    dest[(*j)++] = replacement[1];
}


// --------------------------------------------------------------------
// --------------------------------------------------------------------


/*  NOTE: the transliterate_umlaut function is from claude.ai
*   above is my attempt I tried for days but got nowhere. I dont think I wouldve figured
*   this out I was on the wrong path
*/
char* transliterate_umlaut(const char* input) {
    size_t len = strlen(input);
    char* output = malloc(len * 2 + 1); // Worst case: every char is an umlaut
    size_t j = 0;

    for (size_t i = 0; i < len; ) {
        if ((unsigned char)input[i] == 0xC3) {
            switch ((unsigned char)input[i + 1]) {
                case 0xA4: // ä
                case 0x84: // Ä
                    output[j++] = 'a';
                    output[j++] = 'e';
                    break;
                case 0xB6: // ö
                case 0x96: // Ö
                    output[j++] = 'o';
                    output[j++] = 'e';
                    break;
                case 0xBC: // ü
                case 0x9C: // Ü
                    output[j++] = 'u';
                    output[j++] = 'e';
                    break;
                default:
                    output[j++] = input[i];
                    output[j++] = input[i + 1];
            }
            i += 2;
        } else {
            output[j++] = input[i++];
        }
    }
    output[j] = '\0';
    return output;
}
