#include <stdio.h>
#include <stdlib.h>
#include <caca.h>
#include <curl/curl.h>
#include <errno.h>
#include <string.h>


#include "../include/cJSON.h"
#include "../include/terminal_support.h"


const char *weather_png_filepath = "src/resources/weather.png";
const char *source_filename = "image_to_ascii.c";

size_t image_to_ascii(void)
{

  // this would be kinda sick but this is a cli, maybe put it into code and compile it aswell ?
  //  https://github.com/JosefVesely/img2ascii
  
    /*
     * convert src/resources/weather.png to ascii with libcaca
     * return 0 if success return 1 if not
    */ 

    FILE *weather_png = fopen(weather_png_filepath, "rb");

    if (weather_png == NULL) {
      fprintf(stderr, "Couldn't open weather.png in %s: ERROR: %s", source_filename, strerror(errno));
      return 1;
    }

    // this creates a new windows, but we want to draw in the terminal
    caca_canvas_t *cv; caca_display_t *dp; caca_event_t ev;
    dp = caca_create_display(NULL);
    if(!dp) return 1;
    cv = caca_get_canvas(dp);
    caca_set_display_title(dp, "Hello!");
    caca_set_color_ansi(cv, CACA_BLACK, CACA_WHITE);
    caca_put_str(cv, 0, 0, "This is a message");
    caca_refresh_display(dp);
    caca_get_event(dp, CACA_EVENT_KEY_PRESS, &ev, -1);
    caca_free_display(dp);
    return 0;
}
