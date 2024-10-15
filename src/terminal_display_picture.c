#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

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
    char *tmp_filename = strrchr(url, '/');
    /* If filename is NULL there was no slash found in the url */
    if (tmp_filename == NULL) {
        return NULL;
    }
    // https://stackoverflow.com/questions/4295754/how-to-remove-first-character-from-c-string
    if (tmp_filename[0] == '/') {
        /* copy n bytes from memory area src to memory area dest */
        memmove(/*dest*/ tmp_filename, /*src*/tmp_filename+1, /*size*/strlen(tmp_filename));
    }
    return tmp_filename;
}

/* BUG: Somewhere is heap corruption we attempt to access memory that is not allocated */
/* Only crashes sometimes, but not everytime, about 60% */

// TODO: Copy the files into src/resources folder.

size_t terminal_display_picture(const cJSON *current) 
{
    // int result = 0;
    const cJSON *weather_icons_array_item = NULL;
    const cJSON *weather_icons_array = NULL;
    // char weather_image_filepath[1025];
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
                    CURLcode weather_icon_image_result;

                    CURL *weather_icon_image;
                    weather_icon_image = curl_easy_init();

                    FILE *fp;

                    /*
                     * NOTE: We now have the filename but with slash, gotta get rid of that.
                     * Next I need to implement the caching mechanism.
                     * We should check if the filename is already in src/resources/##.png
                     * If yes, use this, if not download it
                    */

                    char *t_weather_icons_array_item_string = malloc(strlen(weather_icons_array_item->valuestring) + 1);

                    if (t_weather_icons_array_item_string == NULL) {
                        fprintf(stderr, "Memory allocation for %s failed %s", t_weather_icons_array_item_string, strerror(errno));
                    }
                    strcpy(t_weather_icons_array_item_string, (char *)weather_icons_array_item->valuestring);
                    char *t_filename = get_filename(t_weather_icons_array_item_string);

                    if (t_filename == NULL) {
                        fprintf(stderr, "Failed to get filename %s", strerror(errno));
                        printf("\n");
                    }

                    // this was my allocation for filename but this is apparently better
                    // filename = malloc(sizeof(char *) * strlen(t_filename));

                    size_t needed_size = snprintf(NULL, 0, "../src/resources/%s", t_filename) + 1;
                    char *filename = malloc(needed_size);

                    if (filename == NULL) {
                        fprintf(stderr, "Couldn't allocate enough memory for %s %s", filename, strerror(errno));
                        printf("\n");
                    }

                    snprintf(filename, needed_size ,"../src/resources/%s", t_filename);
                    // Check if the file already exists. if no then do this
                    // https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c/230068#230068
                    if (access(filename, F_OK) != 0 && weather_icon_image) {
                        fp = fopen(filename, "wb+");
                        // BUG: For some reason fp seems to be NULL, but picture is there
                        if (fp == NULL) { 
                            fprintf(stderr, "Error opening %s: %s", filename, strerror(errno));
                            printf("\n");
                            return 1;
                        }

                        curl_easy_setopt(weather_icon_image, CURLOPT_URL, weather_icons_array_item->valuestring);
                        curl_easy_setopt(weather_icon_image, CURLOPT_WRITEFUNCTION, NULL);
                        curl_easy_setopt(weather_icon_image, CURLOPT_WRITEDATA, fp);

                        printf("downloaded image\n");
                        weather_icon_image_result = curl_easy_perform(weather_icon_image);
                        if (weather_icon_image_result != 0) {
                            perror("Cannot download image");
                            printf("\n");
                            return 1;
                        }
                    } else if (errno == ENOENT ){
                        fprintf(stderr, "File does not exist in path! %s", strerror(errno));
                        printf("\n");
                        return 1;
                    }
                    free(t_weather_icons_array_item_string);
                    curl_easy_cleanup(weather_icon_image);
                    fclose(fp);
                } else {
                    fprintf(stderr, "weather_icons_array_item. %s", strerror(errno));
                    printf("\n");
                }
            }
        }

    FILE *user_command;
    // CAN FIX MAGIC NUMBERS ?
    char path[1024];
    char tmp_check[1024];

    printf("%s %s", supported_image_viewer, filename);
    snprintf(tmp_check, sizeof(tmp_check), "%s %s > /dev/null 2>&1", supported_image_viewer , filename);
    long user_image_check = system(tmp_check);
    if (user_image_check != 0) {
        perror("Couldn't open picture! Check failed");
        fprintf(stderr, "\n%s %s > /dev/null 2>&1 returned %ld\n", supported_image_viewer, filename, user_image_check);
        return 1;
    }

    // TODO: MAKE FLEXIBLE
    char tmp_test[1024];
    snprintf(tmp_test, sizeof(tmp_test), "%s -b auto %s", supported_image_viewer ,filename);
    user_command = popen(tmp_test, "r");
    if (user_command == NULL) {
        perror("popen failed");
        return 1;
    }

    fprintf(stdout, "\n");
    fprintf(stdout, "         ");


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

    free(filename);

    #endif 

    #ifdef __APPLE__

        fprintf(stdout, "MacOS Detected \n");
        char *get_terminal_emulator_OS_MAC = "echo $TERM_PROGRAM";
        system(get_terminal_emulator_OS_MAC);
        result_terminal_emulator = system(get_terminal_emulator_OS_MAC);

        if (result_terminal_emulator != 0) {
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
