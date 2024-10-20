#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>

#include "../include/cJSON.h"
#include "../include/terminal_support.h"
#include "../include/image_to_ascii.h"


static size_t header_callback(  char *buffer, size_t size, size_t nitems, 
                                void *userdata);

char *get_filename(const char *url);

bool file_exists (char *filename);

size_t terminal_display_picture(const cJSON *current) 
{
    const cJSON *WEATHER_ICONS_ARRAY_ITEM = NULL;
    const cJSON *WEATHER_ICONS_ARRAY = NULL;
    char *filename = NULL;
    char *t_WEATHER_ICONS_ARRAY_ITEM_string = NULL;
    char *supported_image_viewer = "timg";

    #if __linux__

        if (get_terminal_emulator_protocol() != 0) {
            fprintf(stdout, "\n*A Picture of the current weather*\r\n");
            fprintf(stdout, "Your terminal emulator doesn't seem to support displaying pictures.\r\n");
            // image_to_ascii();
        }
	
        if(current == NULL) {
            perror("current is NULL, json file damaged");
            printf("\n");
            exit(1);
        }

        WEATHER_ICONS_ARRAY = cJSON_GetObjectItemCaseSensitive(current, "weather_icons");
        if (cJSON_IsArray(WEATHER_ICONS_ARRAY)) {
            WEATHER_ICONS_ARRAY_ITEM = cJSON_GetArrayItem(WEATHER_ICONS_ARRAY, 0);
            if (cJSON_IsString(WEATHER_ICONS_ARRAY_ITEM) && (WEATHER_ICONS_ARRAY_ITEM->valuestring != NULL)) {
            /* This downloades the picture that the api provides */
                CURLcode weather_icon_image_result = '0';

                CURL *weather_icon_image = NULL;
                weather_icon_image = curl_easy_init();

                FILE *fp = NULL;

                size_t needed_size_t = strlen(WEATHER_ICONS_ARRAY_ITEM->valuestring) + 1;
                t_WEATHER_ICONS_ARRAY_ITEM_string = malloc(needed_size_t);

                if (t_WEATHER_ICONS_ARRAY_ITEM_string == NULL) {
                    fprintf(stderr, "Memory allocation for %s failed %s", 
                            t_WEATHER_ICONS_ARRAY_ITEM_string, strerror(errno));
                }
                strcpy(t_WEATHER_ICONS_ARRAY_ITEM_string, (char *)WEATHER_ICONS_ARRAY_ITEM->valuestring);
                char *t_filename = get_filename(t_WEATHER_ICONS_ARRAY_ITEM_string);

                if (t_filename == NULL) {
                    fprintf(stderr, "Failed to get filename %s", strerror(errno));
                    printf("\n");
                }

                // this was my allocation for filename but this is apparently better
                // filename = malloc(sizeof(char *) * strlen(t_filename));

                // printf("%s\n", t_filename);
                size_t needed_size = snprintf(NULL, 0, "../src/resources/%s", t_filename) + 1;
                filename = malloc(needed_size);

                if (filename == NULL) {
                    fprintf(stderr, "Couldn't allocate enough memory for %s %s", filename, strerror(errno));
                    printf("\n");
                }

                // TODO: IS the filename checked correctly ?

                snprintf(filename, needed_size ,"../src/resources/%s", t_filename);
                // Check if the file already exists. if no then do this
                // https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c/230068#230068


                if (!file_exists(filename) && weather_icon_image) {
                    fp = fopen(filename, "wb+");
                    if (fp == NULL) { 
                        fprintf(stderr, "Error opening %s: %s", filename, strerror(errno));
                        printf("\n");
                        return 1;
                    }

                    curl_easy_setopt(weather_icon_image, CURLOPT_URL, WEATHER_ICONS_ARRAY_ITEM->valuestring);
                    curl_easy_setopt(weather_icon_image, CURLOPT_WRITEFUNCTION, NULL);
                    curl_easy_setopt(weather_icon_image, CURLOPT_WRITEDATA, fp);

                    weather_icon_image_result = curl_easy_perform(weather_icon_image);
                    if (weather_icon_image_result != 0) {
                        perror("Cannot download image");
                        printf("\n");
                        return 1;
                    }
                    fflush(stderr);
                    curl_easy_cleanup(weather_icon_image);
                    fclose(fp);
                } 
            }
        }

    FILE *user_command;
    char path[2048] = {"NULL"};
    char tmp_check[2048] = {"NULL"};

    if (filename == NULL) {
        fprintf(stdout, "filename is NULL before check\n");
    } 

    snprintf(tmp_check, sizeof(tmp_check), "%s %s > /dev/null 2>&1", supported_image_viewer , filename);
    long user_image_check = system(tmp_check);
    if (user_image_check != 0) {
        perror("Couldn't open picture! Check failed");
        // fprintf(stderr, "\n%s %s > /dev/null 2>&1 returned %ld\n", supported_image_viewer, filename, user_image_check);
        return 1;
    }

    // TODO: MAKE FLEXIBLE
    char tmp_test[2048] = {"NULL"};
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
    free(t_WEATHER_ICONS_ARRAY_ITEM_string);

    filename = NULL;
    t_WEATHER_ICONS_ARRAY_ITEM_string = NULL;

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
                    WEATHER_ICONS_ARRAY_ITEM = cJSON_GetArrayItem(weather_icons_array, 0);
                    if (cJSON_IsString(WEATHER_ICONS_ARRAY_ITEM) && (WEATHER_ICONS_ARRAY_ITEM->valuestring != NULL)) {
                        // snprintf(command, sizeof(command), "%s %s", user_image_viewer, WEATHER_ICONS_ARRAY_ITEM->valuestring);
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


//https://curl.se/libcurl/c/CURLOPT_HEADERFUNCTION.html
static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
  return nitems * size;
}


char *get_filename(const char *url)
{
    // find the last occurence of "/" in the url. with strrchr()
    char *filename = strrchr(url, '/');
    /* If filename is NULL there was no slash found in the url */
    if (filename == NULL) {
        return NULL;
    }
    // https://stackoverflow.com/questions/4295754/how-to-remove-first-character-from-c-string
    if (filename[0] == '/') {
        /* copy n bytes from memory area src to memory area dest */
        memmove(/*dest*/ filename, /*src*/filename+1, /*size*/strlen(filename));
    }
    return filename;
}

// https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c
bool file_exists (char *filename)
{
  struct stat   buffer;
  return (stat (filename, &buffer) == 0);
}


