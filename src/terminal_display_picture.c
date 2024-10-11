#include <stdio.h>
#include <stdlib.h>
#include <caca.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <errno.h>
#include <string.h>

#include "../include/cJSON.h"
#include "../include/terminal_support.h"
#include "../include/image_to_ascii.h"


/* This function will check if you have the terminal image viewers installed,
 * and if your terminal emulator can display a picture, if not you will be shown
 * ascii.
 */

const char *tmp_weather_png_filename = "src/resources/weather.png";

size_t terminal_display_picture(const cJSON *current) 
{

    // TODO: Determine if a cli of is installed, 
    // if there is no cli there or the terminal cant display the picture try ascii
    //
    /* Terminal Image viewers 
     *  timg
     *  chafa
    */

    char command[1024];
    int result = 0;
    const cJSON *weather_icons_array_item = NULL;
    const cJSON *weather_icons_array = NULL;

    // These are all the image viewers that are "supported"
    char *supported_image_viewers[] = { "timg" , "chafa" , NULL};
    // This is the image viewer that is both on the user system and installed
    char *user_image_viewer = "zero";


    // NOTE: Alactritty seems to show the image but at a really bad quality
    char *supported_terminals[] = { "ghostty", "kitty", "wezterm" , NULL };


    // TODO: With the terminal emulator name we got check against the array, if its in there pass it to the command


    /* We are magically checking if any of the imager viewers listed in the array are installed on the system */
    for (int i = 0; supported_image_viewers[i] != NULL ;i++) {
        // run a test command for the supported image viewers, check the output if its good assign user_image_viewer
        char * tmp_image_viewer;

        snprintf(command, sizeof(command), "%s > /dev/null", supported_image_viewers[0]);
        result = system(command);

	/* Search for the program in the PATH  kinda like which $s , supported_image_viewers[i] */
	// https://stackoverflow.com/questions/41230547/check-if-program-is-installed-in-c
        
        if (result != 0) {
            perror("supported_image_viewers command returned something wrong/bad idk.\n");
            break;
        }
	
	tmp_image_viewer = supported_image_viewers[0];
        user_image_viewer = tmp_image_viewer ;
    }


    // smth like this: https://askubuntu.com/questions/210182/how-to-check-which-terminal-emulator-is-being-currently-used
    // int result_terminal_emulator;

    // der output von get_terminal_emulator_OS_MAC muss gegen den array von unterstützen terminals überprüft werden. 
    // und dann muss man noch schauen welche image_vewer der user hat und dann das beide in den command gepumpt werden

    // TODO:  https://stackoverflow.com/questions/646241/c-run-a-system-command-and-get-output

    // const char *ascii_image_filepath = "src/resources/ascii.png";


    #if __linux__
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

                    if (weather_icon_image) {
                        fp = fopen(tmp_weather_png_filename, "wb+");
                        if (fp == NULL) { 
                            fprintf(stderr, "Error opening %s : %s", tmp_weather_png_filename, strerror(errno));
                        }

                        curl_easy_setopt(weather_icon_image, CURLOPT_URL, weather_icons_array_item->valuestring);
                        curl_easy_setopt(weather_icon_image, CURLOPT_WRITEFUNCTION, NULL);
                        curl_easy_setopt(weather_icon_image, CURLOPT_WRITEDATA, fp);

                        weather_icon_image_result = curl_easy_perform(weather_icon_image);
                        if (weather_icon_image_result != 0) {
                            perror("Cannot download image\n");
                        }
                    }

                    // snprintf(command, sizeof(command), "%s %s", user_image_viewer, tmp_weather_png_filename);
                    // result = system(command);

                    curl_easy_cleanup(weather_icon_image);
                    fclose(fp);
                } else {
                    fprintf(stderr, "weather_icons_array_item ERROR: Is not valuestring\n");
                }
            }
        }

	//printf("\033[1B");
	printf("\n");
	printf("         ");
        FILE *user_command;
        char path[1024];

        // timg wird dann ersetzt durch user_image_viewer
        // snprintf(input, sizeof(input), "%s %s", user_image_viewer, tmp_weather_png_filename);
        // user_command = popen(input);

	int test = system("timg src/resources/weather.png > /dev/null");
	if (test != 0) {
	    perror("Couldn't open picture check failed");
	    printf("\n");
	    return 1;
	}
	
        user_command = popen("timg src/resources/weather.png", "r");

        while (fgets(path, sizeof(path), user_command) != NULL) {
           fprintf(stdout, "%s", path);
        }
        pclose(user_command);

        if (get_terminal_emulator_protocol() !=0L) {
            image_to_ascii();
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
