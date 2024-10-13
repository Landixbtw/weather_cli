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

    char command[1024];
    int result = 0;
    const cJSON *weather_icons_array_item = NULL;
    const cJSON *weather_icons_array = NULL;

    // replace the tmp image filename with the whole ass download path for the picture with src/resources/***.png
    char *supported_image_viewer = "timg";

    #if __linux__

        if (get_terminal_emulator_protocol() !=0 ) {
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

                    if (weather_icon_image) {
                        fp = fopen(tmp_weather_png_filename, "wb+");
                        if (fp == NULL) { 
                            fprintf(stderr, "Error opening %s : %s\n", tmp_weather_png_filename, strerror(errno));
                        }

                        curl_easy_setopt(weather_icon_image, CURLOPT_URL, weather_icons_array_item->valuestring);
                        curl_easy_setopt(weather_icon_image, CURLOPT_WRITEFUNCTION, NULL);
                        curl_easy_setopt(weather_icon_image, CURLOPT_WRITEDATA, fp);

                        weather_icon_image_result = curl_easy_perform(weather_icon_image);
                        if (weather_icon_image_result != 0) {
                            perror("Cannot download image\n");
                        }
                    }

                    curl_easy_cleanup(weather_icon_image);
                    fclose(fp);
                } else {
                    fprintf(stderr, "weather_icons_array_item ERROR: Is not valuestring\n");
                }
            }
        }

	//printf("\033[1B");
        FILE *user_command;
	// CAN FIX MAGIC NUMBERS ?
	char path[1024];
	char tmp_check[1024];

	snprintf(tmp_check, sizeof(tmp_check), "timg %s > /dev/null ", tmp_weather_png_filename);
	int user_image_check = system(tmp_check);
	if (user_image_check != 0) {
	    perror("Couldn't open picture check failed.");
	    printf("\n");
	    return 1;
	}

	// TODO: MAKE FLEXIBLE
        user_command = popen("timg -b auto src/resources/weather.png", "r");

	printf("\n");
	printf("         ");

	while (fgets(path, sizeof(path), user_command) != NULL) {
           fprintf(stdout, "%s", path);
        }
        pclose(user_command);

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
