

#ifndef  TERMINAL_SUPPORT_H_
#define TERMINAL_SUPPORT_H_

#include "cJSON.h"
#include <stdio.h>

char *get_terminal_emulator_name(void);
size_t terminal_display_picture(const cJSON *current);

#endif
