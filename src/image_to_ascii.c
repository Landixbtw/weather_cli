#include <stdio.h>
#include <stdlib.h>
// #include <caca.h>
#include <curl/curl.h>
#include <errno.h>
#include <string.h>


#include "../include/cJSON.h"
#include "../include/terminal_support.h"


const char *weather_png_filepath = "src/resources/weather.png";
const char *ascii_png_filepath = "src/resources/ascii.png";

size_t image_to_ascii(void)
{

  // this would be kinda sick but this is a cli, maybe put it into code and compile it aswell ?
  //  https://github.com/JosefVesely/img2ascii

    /*
     * convert src/resources/weather.png to ascii with libcaca
     * return 0 if success return 1 if not
    */

    if (get_terminal_emulator_name() != 0) {
        FILE *weather_png = fopen(weather_png_filepath, "rb");
	// FIX: MAGIC NUMBERS
	char path[1024];
        char input[2048] ;
        FILE *img2ascii_command;

        if (weather_png == NULL) {
          fprintf(stderr, "Couldn't open weather.png ERROR: %s",  strerror(errno));
          exit(EXIT_FAILURE);
        }

	char *tmp_command = "./img2ascii/img2ascii > /dev/null";
	int result = system(tmp_command);
	if (result == -1 ) {
	  fprintf(stderr, "Cannot open the img2ascii cli tool\n");
	}

        snprintf(input, sizeof(input), "./img2ascii/img2ascii --input %s --output %s", weather_png_filepath, ascii_png_filepath);
	img2ascii_command = popen(input, "r");

        while (fgets(path, sizeof(path), img2ascii_command) != NULL) {
            fprintf(stdout, "%s", path);
        }

        return 0;
    }
    return 1;
}
