
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <wchar.h>
#include <locale.h>
#include <unistd.h>

#include <curl/curl.h>
#include <curl/easy.h>

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


size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
void build_url(char *CITY);
size_t terminal_display_picture(const cJSON *current);
//
void replace_umlaute(char *dest, wchar_t umlaut, size_t *j);
char *filter_char(const wchar_t input, size_t output_size);
//
char* transliterate_umlaut(const char* input);
char *get_terminal_emulator_name(void);

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

    if (!setlocale(LC_CTYPE, "de_DE.UTF-8")) {
        fprintf(stderr, "Can't set the specified locale!\n Check LANG, LC_CTYPE, LC_ALL.\n");
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
        temp_json_file = fopen("json_data.json", "wb+");

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
    const cJSON *weather_icons = NULL;

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
            fprintf(stdout, "- Observation Time: %s.\n", observation_time->valuestring);
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
            fprintf(stdout, "- Humidity: %i%%.\n", humidity->valueint);
        }

        feelslike = cJSON_GetObjectItemCaseSensitive(current, "feelslike");

        if (cJSON_IsNumber(feelslike)) {
            fprintf(stdout, "- Feels like: %i°C.\n", feelslike->valueint);
        }

        if (&terminal_display_picture) {
            terminal_display_picture(current);
        }
    }

    // show the picture if possible

    fclose(read_api_key_file);
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
     * "I know this void * pointer is actually pointing to a FILE structure, so it's safe to treat it as a FILE * "
     *
     * NOTE: I got help from claude, explaining this function. I obviously, 
     * took it from the libcurl website but I understand it thanks to claude explaining it.
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
    const cJSON *weather_icons_array_item = NULL;
    const cJSON *weather_icons_array = NULL;
    char *command = malloc(sizeof(char *) * 124);
    const char *image_viewers[] = {"timg" , "chafa"};

    // NOTE: Alactritty seems to show the image but at a really bad quality
    const char *supported_terminals[] = { "ghostty", "kitty", "wezterm"};

    // TODO: Check with terminal emulator the user is using
    //
    // smth like this: https://askubuntu.com/questions/210182/how-to-check-which-terminal-emulator-is-being-currently-used

    int result_terminal_emulator;

    // der output von get_terminal_emulator_OS_MAC muss gegen den array von unterstützen terminals überprüft werden. 
    // und dann muss man noch schauen welche image_vewer der user hat und dann das beide in den command gepumpt werden

    // TODO:  https://stackoverflow.com/questions/646241/c-run-a-system-command-and-get-output

    char *user_image_viewer;
    char *terminal_emulator_name_OS_LINUX = get_terminal_emulator_name();

    #if __linux__
        if(current != NULL) {
            weather_icons_array = cJSON_GetObjectItemCaseSensitive(current, "weather_icons");
            if (cJSON_IsArray(weather_icons_array)) {
                weather_icons_array_item = cJSON_GetArrayItem(weather_icons_array, 0);
                if (cJSON_IsString(weather_icons_array_item) && (weather_icons_array_item->valuestring != NULL)) {
                    // snprintf(command, sizeof(command), "%s %s", user_image_viewer, weather_icons_array_item->valuestring);
                    // result = system(command);
                }
            }
        }

        if (result == -1) {
            fprintf(stderr, "Couldn't open image.\n");
            return 1;
        } else {
            // printf("%ld\n", result);
            // system(command);
        }
    #endif 

    #ifdef __APPLE__

        fprintf(stdout, "MacOS Detected \n");
        char *get_terminal_emulator_OS_MAC = "echo $TERM_PROGRAM";

        // der output von get_terminal_emulator_OS_MAC muss gegen den array von unterstützen terminals überprüft werden. 
        // und dann muss man noch schauen welche image_vewer der user hat und dann das beide in den command gepumpt werden
        // und 

        char *terminal_emulator_name_OS_MAC;
        system(get_terminal_emulator_OS_MAC);

        result_terminal_emulator = system(get_terminal_emulator_OS_MAC);

        if (result_terminal_emulator == -1) {
            fprintf(stderr, "Couldn't determine terminal emulator");
            return 1;
        } else {
            if(current != NULL) {
                weather_icons_array = cJSON_GetObjectItemCaseSensitive(current, "weather_icons");
                if (cJSON_IsArray(weather_icons_array)) {
                    weather_icons_array_item = cJSON_GetArrayItem(weather_icons_array, 0);
                    if (cJSON_IsString(weather_icons_array_item) && (weather_icons_array_item->valuestring != NULL)) {
                        snprintf(command, sizeof(command), "%s %s", user_image_viewer, weather_icons_array_item->valuestring);
                        result = system(command);
                    }
                }
            }

            if (result == -1) {
                fprintf(stderr, "Couldn't open image.\n");
                return 1;
            } else {
                system(command);
                printf("%s", command);
            }
        }
    #endif

    return 0;
}


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

    char *output;

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

/* This is 50/50 me and claude.ai */
char *get_terminal_emulator_name(void)
{
    char *terminal_name;

    /*
    * We can start by getting the parent process ID of our program.
    * Then, we can read the command line of that parent process from the /proc filesystem.
    * The command line often (but not always) includes the name of the terminal emulator.
    */

    // *p*arent *p*rocess *id*
    pid_t ppid = getppid();

    // TODO: This gives the shell not the terminal emulator name 

    char proc_path[256];

    snprintf(proc_path, sizeof(proc_path), "/proc/%d/cmdline", ppid);
    printf("proc_path: %s\n", proc_path);


    FILE *f = fopen(proc_path, "r");
    if (f == NULL) {
        perror("Failed to open proc file\n");
        return NULL;
    }


    char *line = NULL;
    size_t len = 0;
    ssize_t read = getline(&line, &len, f);
    fclose(f);

    if (read == -1) {
        free(line);
        return NULL;
    }

    // Extract the terminal emulator name from the command line
    // This part will depend on the specific format of your system
    terminal_name = strrchr(line, '/');
    if (terminal_name) {
        terminal_name++; // Move past the '/'
    } else {
        terminal_name = line;
    }

    // Remove any arguments
    char *space = strchr(terminal_name, ' ');
    if (space) *space = '\0';

    printf("terminal name: %s\n", terminal_name);
    return terminal_name;
}



