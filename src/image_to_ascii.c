#include <stdio.h>
#include <stdlib.h>
#include <caca.h>
#include <curl/curl.h>

#include "../include/cJSON.h"
#include "../include/terminal_support.h"


size_t image_to_ascii(void)
{
    return 0;
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

    int result = 0;
    const cJSON *weather_icons_array_item = NULL;
    const cJSON *weather_icons_array = NULL;
    char *command = malloc(sizeof(char *) * 124);

    // These are all the image viewers that are "supported"
    char *supported_image_viewers[] = { "timg" , "chafa" , NULL};
    // This is the image viewer that is both on the user system and installed
    char *user_image_viewer;


    // NOTE: Alactritty seems to show the image but at a really bad quality
    char *supported_terminals[] = { "ghostty", "kitty", "wezterm" , NULL };


    // TODO: With the terminal emulator name we got check against the array, if its in there pass it to the command


    /* We are magically checking if any of the imager viewers listed in the array are installed on the system */
    for (int i = 0; supported_image_viewers[i] != NULL ;i++) {
        // run a test command for the supported image viewers, check the output if its good assign user_image_viewer
        char * tmp_image_viewer;

        // snprintf(command, sizeof(command), "%s", supported_image_viewers[i]);
        // result = system(command);

        tmp_image_viewer = supported_image_viewers[0];
        if (result == -1) {
            perror("supported_image_viewers command returned something wrong/bad idk.\n");
            break;
        }

        user_image_viewer = tmp_image_viewer ;
    }


    // smth like this: https://askubuntu.com/questions/210182/how-to-check-which-terminal-emulator-is-being-currently-used
    int result_terminal_emulator;

    // der output von get_terminal_emulator_OS_MAC muss gegen den array von unterstützen terminals überprüft werden. 
    // und dann muss man noch schauen welche image_vewer der user hat und dann das beide in den command gepumpt werden

    // TODO:  https://stackoverflow.com/questions/646241/c-run-a-system-command-and-get-output

    char *terminal_emulator_name_OS_LINUX = get_terminal_emulator_name();
    char *tmp_weather_png_filename = "src/resources/weather.png";

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
                        fp = fopen(tmp_weather_png_filename, "wb");
                        if (fp == NULL) {
                            perror("Error opening src/resources/weather.png.\n");
                        }

                        curl_easy_setopt(weather_icon_image, CURLOPT_URL, weather_icons_array_item->valuestring);
                        curl_easy_setopt(weather_icon_image, CURLOPT_WRITEFUNCTION, NULL);
                        curl_easy_setopt(weather_icon_image, CURLOPT_WRITEDATA, fp);

                        weather_icon_image_result = curl_easy_perform(weather_icon_image);
                        if (weather_icon_image_result) {
                            perror("Cannot download image\n");
                        }
                    }

                    // snprintf(command, sizeof(command), "%s %s", user_image_viewer, tmp_weather_png_filename);
                    // result = system(command);

                    curl_easy_cleanup(weather_icon_image);
                    fclose(fp);
                }
            }
        }

        if (result == -1) {
            fprintf(stderr, "Couldn't open image.\n");
            return 1;
        } else {
            FILE *user_command;
            char path[1024];

            char *input;
            // timg wird dann ersetzt durch user_image_viewer
            // snprintf(input, sizeof(input), "%s %s", user_image_viewer, tmp_weather_png_filename);
            user_command = popen("timg src/resources/weather.png", "r");

            if (!user_command) {
                perror("Couldnt execute command");
                return 1;
            }

            while (fgets(path, sizeof(path), user_command) != NULL) {
                fprintf(stdout, "%s", path);
            }

            pclose(user_command);
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
