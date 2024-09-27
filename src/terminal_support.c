#include "cJSON.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


/* This is 50/50 me and claude.ai */
char *get_terminal_emulator_name(void)
{
    /*
    * We can start by getting the parent process ID of our program.
    * Then, we can read the command line of that parent process from the /proc filesystem.
    * The command line often (but not always) includes the name of the terminal emulator.
    */

    char *terminal_name;
    // *p*arent *p*rocess *id*
    pid_t ppid = getppid();

    // TODO: This gives the shell not the terminal emulator name 

    #ifdef __LINUX__
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

    #endif 


    #ifdef __APPLE__ 

        fprintf(stdout, "PPID: %i\n", ppid);
        printf("PID APPLE: %i\n", getpid());

    #endif

    return terminal_name;
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

    // These are all the image viewers that are "supported"
    const char *supported_image_viewers[] = { "timg" , "chafa" };
    // This is the image viewer that is both on the user system and installed
    char *user_image_viewer;


    // NOTE: Alactritty seems to show the image but at a really bad quality
    const char *supported_terminals[] = { "ghostty", "kitty", "wezterm" };


    // TODO: With the terminal emulator name we got check against the array, if its in there pass it to the command


    /* We are magically checking if any of the imager viewers listed in the array are installed on the system */
    for (int i = 0;  i < sizeof(supported_image_viewers); i++) {
        // run a test command for the supported image viewers, check the output if its good assign user_image_viewer
        char * tmp_img_vwr;
        char *command;
        int result = 0;


        // BUG: This line is printing "feelslike" as an image viewer although its not in the array
        // BUG: This line is causing a segfault
        //
        // output is 
        // timg
        // chafa
        // feelslike
        // segfault
        //
        printf("%s\n", supported_image_viewers[i]);
        // snprintf(command, sizeof(command), "%s", supported_image_viewers[i]);
        // result = system(command);

        if (result == -1) {
            fprintf(stderr, "bla bla bla errorcode: %i\n", result);
            break;
        }

        // tmp_img_vwr = user_image_viewer;
    }

    // smth like this: https://askubuntu.com/questions/210182/how-to-check-which-terminal-emulator-is-being-currently-used

    int result_terminal_emulator;

    // der output von get_terminal_emulator_OS_MAC muss gegen den array von unterstützen terminals überprüft werden. 
    // und dann muss man noch schauen welche image_vewer der user hat und dann das beide in den command gepumpt werden

    // TODO:  https://stackoverflow.com/questions/646241/c-run-a-system-command-and-get-output

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
