#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <errno.h>
#include <string.h>


#include "../include/terminal_support.h"


const char *WEATHER_PNG_FILEPATH = "src/resources/weather.png";
const char *ASCII_PNG_FILEPATH = "src/resources/ascii.png";

size_t image_to_ascii(void)
{

  // this would be kinda sick but this is a cli, maybe put it into code and compile it aswell ?
  //  https://github.com/JosefVesely/img2ascii

    // ascii-image-converter file/path -C
    // https://github.com/TheZoraiz/ascii-image-converter

    /*
     * convert src/resources/weather.png to asciia
     * return 0 if success return 1 if not
    */

    if (get_terminal_emulator_protocol() != 0) {
        FILE *weather_png = fopen(WEATHER_PNG_FILEPATH, "rb");
        // FIX: MAGIC NUMBERS
        char path[1024] = {"0"};
        char input[2048] = {"0"};
        char tmp_input[2048] = {"0"};
        FILE *img2ascii_command;

        if (weather_png == NULL) {
        fprintf(stderr, "Couldn't open %s \nERROR: %s\n", WEATHER_PNG_FILEPATH, strerror(errno));
            exit(EXIT_FAILURE);
        }

        const char *TMP_COMMAND = "./img2ascii/img2ascii";
        snprintf(tmp_input, sizeof(tmp_input), "./img2ascii/img2ascii --input %s --output %s > /dev/null", WEATHER_PNG_FILEPATH, ASCII_PNG_FILEPATH);

        int result = system(TMP_COMMAND);
        if (result != 0 ) {
            fprintf(stderr, "Cannot open the img2ascii cli tool\n");
        }

        snprintf(input, sizeof(input), "./img2ascii/img2ascii --print --input %s", WEATHER_PNG_FILEPATH);
        img2ascii_command = popen(input , "r");

        while (fgets(path, sizeof(path), img2ascii_command) != NULL) {
            fprintf(stdout, "%s", path);
        }

        fclose(img2ascii_command);
        fclose(weather_png);
        printf("\n");
        return 0;
    }
    return 1;
}
