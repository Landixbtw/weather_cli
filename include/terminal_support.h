

#ifndef  TERMINAL_SUPPORT_H_
#define TERMINAL_SUPPORT_H_

#include "cJSON.h"
#include <stdio.h>

size_t get_terminal_emulator_protocol(void);
size_t terminal_display_picture(const cJSON *current);

#endif
