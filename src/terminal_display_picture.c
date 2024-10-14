#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <errno.h>
#include <string.h>

#include "../include/cJSON.h"
#include "../include/terminal_support.h"
#include "../include/image_to_ascii.h"


//https://curl.se/libcurl/c/CURLOPT_HEADERFUNCTION.html
static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
  return nitems * size;
}

char *get_filename(const char *url)
{
    // find the last occurence of "/" in the url. with strrchr()
    char *t_filename = strrchr(url, '/');
    /* If filename is NULL there was no slash found in the url */
    if (t_filename == NULL) {
        return NULL;
    }
    // https://stackoverflow.com/questions/4295754/how-to-remove-first-character-from-c-string
    if (t_filename[0] == '/') {
        /* copy n bytes from memory area src to memory area dest */
        memmove(/*dest*/ t_filename, /*src*/t_filename+1, /*size*/strlen(t_filename));
    }
    return t_filename;
}

const char *tmp_weather_png_filename = "src/resources/weather.png";

size_t terminal_display_picture(const cJSON *current) 
{
    int result = 0;
    const cJSON *weather_icons_array_item = NULL;
    const cJSON *weather_icons_array = NULL;
    char weather_image_filepath[1025];
    char *filename;
    // replace the tmp image filename with the whole ass download path for the picture with src/resources/***.png
    char *supported_image_viewer = "timg";

    #if __linux__

        if (get_terminal_emulator_protocol() !=0) {
            fprintf(stdout, "\n*A Picture of the current weather*\r\n");
            fprintf(stdout, "Your terminal emulator doesn't seem to support displaying pictures.\r\n");
            // image_to_ascii();
        }

        if(current != NULL) {
            weather_icons_array = cJSON_GetObjectItemCaseSensitive(current, "weather_icons");
            if (cJSON_IsArray(weather_icons_array)) {
                weather_icons_array_item = cJSON_GetArrayItem(weather_icons_array, 0);
                if (cJSON_IsString(weather_icons_array_item) && (weather_icons_array_item->valuestring != NULL)) {
                /* This downloades the picture that the api provides */
                    CURL *weather_icon_image;
                    CURLcode weather_icon_image_result;
                    weather_icon_image = curl_easy_init();
                    FILE *fp;
                    // sprintf(weather_image_filepath,"/src/resources/%s", weather_icons_array_item->valuestring);
                    //printf("%s", weather_image_filepath);


                    /*
                     * NOTE: We now have the filename but with slash, gotta get rid of that.
                     * Next I need to implement the caching mechanism.
                     * We should check if the filename is already in src/resources/##.png
                     * If yes, use this, if not download it
                    */

                    char *t_weather_icons_array_item_string = malloc(strlen(weather_icons_array_item->valuestring) + 1);
                    if (t_weather_icons_array_item_string == NULL) {
                        fprintf(stderr, "Memory allocation for t_weather_icons_array_item_string failed :: %s", strerror(errno));
                    }
                    strcpy(t_weather_icons_array_item_string, (char *)weather_icons_array_item->valuestring);
                    char *t_filename = get_filename(t_weather_icons_array_item_string);

                    if (t_filename != NULL) {
                        fprintf(stderr, "filename: %s\n", t_filename);
                    } else {
                        fprintf(stderr, "Failed to get filename\n");
                    }

                    filename = malloc(sizeof(strlen(t_filename)));
                    snprintf(filename, strlen(t_weather_icons_array_item_string) + strlen("src/resources/"),"src/resources/%s", t_filename);
                    if (weather_icon_image ) {
                        fp = fopen(filename, "wb+");
                        if (fp == NULL) { 
                            fprintf(stderr, "Error opening %s :: %s\n", filename, strerror(errno));
                        }
                        curl_easy_setopt(weather_icon_image, CURLOPT_URL, weather_icons_array_item->valuestring);
                        curl_easy_setopt(weather_icon_image, CURLOPT_WRITEFUNCTION, NULL);
                        curl_easy_setopt(weather_icon_image, CURLOPT_WRITEDATA, fp);

                        weather_icon_image_result = curl_easy_perform(weather_icon_image);
                        if (weather_icon_image_result != 0) {
                            perror("Cannot download image\n");
                        }
                    }

                    free(t_weather_icons_array_item_string);
                    curl_easy_cleanup(weather_icon_image);
                    fclose(fp);
                } else {
                    fprintf(stderr, "weather_icons_array_item :: %s\n", strerror(errno));
                }
            }
        }

    //printf("\033[1B");
    FILE *user_command;
    // CAN FIX MAGIC NUMBERS ?
    char path[2048];
    char _check[1024];

    snprintf(_check, sizeof(_check), "timg %s > /dev/null 2>&1", filename);
    int user_image_check = system(_check);
    if (user_image_check != 0) {
        perror("Couldn't open picture! Check failed.");
        fprintf(stdout, "\n");
        return 1;
    }

    // TODO: MAKE FLEXIBLE
    char _test[1025];
    snprintf(_test, sizeof(_test), "timg -b auto %s", filename);
    user_command = popen(_test, "r");
    if (user_command == NULL) {
        perror("popen failed.");
        return 1;
    }

    fprintf(stdout, "\n");
    fprintf(stdout, "         ");

    // FIX: Programm crashes here
    /*
     *corrupted size vs. prev_size
     * sh: : Zeile 3: Dateiende beim Suchen nach »"« erreicht.
     * [1]    64478 IOT instruction (core dumped)  ./main new+york
    */


    while (fgets(path, sizeof(path), user_command) != NULL) {
        fprintf(stdout, "%s", path);
    }

    // the error handling was suggested by claude.ai for troubleshooting 
    // corrupted size vs. prev_size
    // [1]    66015 IOT instruction (core dumped)  ./main new+york

    if (ferror(user_command)) {
        perror("Error reading from pipe");
    }

    int close_result = pclose(user_command);
    if (close_result == -1) {
        perror("pclose failed");
    } else if (WIFEXITED(close_result)) {
        int exit_status = WEXITSTATUS(close_result);
        if (exit_status != 0) {
            fprintf(stderr, "Command exited with status %d\n", exit_status);
        }
    } else if (WIFSIGNALED(close_result)) {
        fprintf(stderr, "Command terminated by signal %d\n", WTERMSIG(close_result));
    }

    //pclose(user_command);
    free(filename);

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
                        // snprintf(command, sizeof(command), "%s %s", user_image_viewer, weather_icons_array_item->valuestring);
                        // result = system(command);
                        result = -1;
                    }
                }
            }

            if (result == -1) {
                fprintf(stderr, "Couldn't open image.\n");
                return 1;
            } else {
                // system(command);
                // printf("%s", command);
            }
        }
    #endif

return 0;
}
