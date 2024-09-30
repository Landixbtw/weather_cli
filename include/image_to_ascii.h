#ifndef IMAGE_TO_ASCII_H_
#define IMAGE_TO_ASCII_H_

#include <stdlib.h>
#include "../include/cJSON.h"

size_t image_to_ascii(void);
size_t terminal_display_picture(const cJSON *current);

#endif
